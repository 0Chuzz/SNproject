/*
 * kalman2.c
 *
 *  Created on: 08/set/2014
 *      Author: alessandro
 */
#include "utils.h"
#include "kalman2.h"
#include "labirinthlabeler.h"

#define KALMAN_OPTIMIZE 1
#define PIC_VERSION
#define MOTION_NOISE 0.1
#define MEASUREMENT_NOISE 0.1

#define VISIBLE_TRESHOLD 2
#define DELETION_TRESHOLD 2

static short freeCount = 0;
static short freed[NUM_BLOBS_MAX];
static short lastUsed = 0;

/** Call before start using Kalman Filter. Used to manage the identifiers of the tracks */
void initialize() {
	freeCount = 0;
	lastUsed = 0;
}
/* Assigns an id to a new track, possibly reusing ids of previously deleted tracks.*/
int assign() {
	if(freeCount > 0) {
		freeCount--;
		return freed[freeCount];
	}
	lastUsed = (lastUsed + 1) % NUM_BLOBS_MAX;
	return lastUsed-1;
}
/**
 * Matrix Product
 */
static inline void matrixproduct(float m1[4][4], float m2[4][4], float result[4][4]) {
	int i,j,k;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			result[i][j] = 0;
			for(k = 0; k < 4; k++)
				result[i][j] += result[i][k] * result[k][j];
		}
	}
}


/**
 * Makes a new prediction for the track t
 */

/** Uses aux. space O(N) and 128 operations */
static inline void unoptimizedPredictCovariance(float cov[4][4]) {
	float temp[4][4];
	matrixproduct( transitionMatrix, cov, temp);
	matrixproduct(cov, transitionMatrixT, cov);
}
/** Uses aux. space O(N) and 32 operations */
static inline void optimizedPredictCovariance(float cov[4][4]) {
	float temp[4][4];
	int i, j;
	//copy
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			temp[i][j] = cov[i][j];
	//First row. Check F * cov * F^T to see correctness
	cov[0][0] = temp[0][0] + temp[0][2] + temp[2][0] + temp[2][2];
	cov[0][1] = temp[0][1] + temp[0][3] + temp[2][1] + temp[2][3];
	cov[0][2] = temp[0][2] + temp[2][2];
	cov[0][3] = temp[0][3] + temp[2][3];

	cov[1][0] = temp[1][0] + temp[1][2] + temp[3][0] + temp[3][2];
	cov[1][1] = temp[1][1] + temp[1][3] + temp[3][1] + temp[3][3];
	cov[1][2] = temp[1][2] + temp[3][2];
	cov[1][3] = temp[1][3] + temp[3][3];

	cov[2][0] = temp[2][0] + temp[2][2];
	cov[2][1] = temp[2][1] + temp[2][3];
	//2,2 and 2,3 don't change (in this part we have identity in F)

	cov[3][0] = temp[3][0] + temp[3][2];
	cov[3][1] = temp[3][1] + temp[3][3];
	//again, 3,2 and 3,3 don't change.
}

void predict(kalmanTrack* t) {
	/** Update exteem of the position */
	t->posX += t->velX;
	t->posY += t->velY;
	//Speed has not to be updated in our model (o.w. introduce acc. todo)
	/** Update covariances */
	#ifdef KALMAN_OPTIMIZE
	optimizedPredictCovariance(t->cov);
	#else
	unoptimizedPredictCovariance(t->cov);
	#endif
	int i;
	for(i = 0; i < 4; i++) { t->cov[i][i] += MOTION_NOISE;} //Q is only diagonal.
	//P = F*P*F^T + Q
}
/** Deletes a track */
int delete(int size, kalmanTrack states[NUM_BLOBS_MAX], int pos) {
	int i;
	freed[freeCount] = states[pos].id;
	freeCount++;
	for(i = pos; i+1 < size; i++) {
		states[i] = states[i+1];
	}
	return size -1;
}
/**
 * Eliminates unused tracks and predicts for the remaining ones.
 */
int predictAll(int size, kalmanTrack states[NUM_BLOBS_MAX]) {
	int i;
	for(i = 0; i < size; i++) {
		if(states[i].consecutiveInvisibleCount++ >= DELETION_TRESHOLD) {
			size = delete(size, states, i); //delete is linear, could be better. But we don't care.
			i--;
		}
		predict(states+i);
		states[i].age++;
	}
	return size;
}


/**
 * Performs a correction of the track based on the measure
 * */

void update(kalmanTrack* t, point measure) {
	float temp[4][4];
	int i, j;
	/** Vect is Y */

	int y[2] = { measure.X - t->posX, measure.Y - t->posY};
	//Temp = S = H x Pk x H^t + R (H is I_2)
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			if(i > 2 || j > 2) temp[i][j] = 0;
			else temp[i][j] = (t->cov[i][j] + measurementNoiseM[i][j]);
		}

	}

	float gain[4][2] = {
			{t->cov[0][0] * temp[1][1] - t->cov[0][1]*temp[1][0], t->cov[0][1] * temp[0][0] - t->cov[0][0]* temp[0][1]},
			{t->cov[1][0] * temp[1][1] - t->cov[1][1]*temp[1][0], t->cov[1][1] * temp[0][0] - t->cov[1][0]*temp[0][1]},
			{t->cov[2][0] * temp[1][1] - t->cov[2][1]*temp[1][0], t->cov[2][1] * temp[0][0] - t->cov[2][0]*temp[0][1]},
			{t->cov[3][0] * temp[1][1] - t->cov[3][1]*temp[1][0], t->cov[3][1] * temp[0][0] - t->cov[3][0]*temp[0][1]}
	};

	float l = 1/((temp[0][0]*temp[1][1]) - (temp[0][1]*temp[1][0]));

	for(i = 0; i < 4; i++)
		for(j = 0; j < 2; j++)	gain[i][j] = gain[i][j] * l;
	//Now gain is K, use it to correct.
	t->posX += gain[0][0]*y[0] + gain[0][1]*y[1];
	//myprintf("Y updates as %d %.5f %d %.5f %d\n", t->posY, gain[1][0], y[0], gain[1][1], y[1]);
	int Y_update = (int)(gain[1][0]*y[0] + gain[1][1]*y[1]);
	//myprintf("Y upd %.5f \n", gain[1][0]*y[0] + gain[1][1]*y[1]);
	t->posY += Y_update;
	t->velX += gain[2][0]*y[0] + gain[2][1]*y[1];
	t->velY += gain[3][0]*y[0] + gain[3][1]*y[1];

	//Update the covariance matrix.

	//temp is now (I-KH) ->  Identity - gain*I_2
	//TODO: check if there's need to optimize or if the compiler will take car.e
	 for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			if(j == i && i < 2) temp[i][j] = 1 - gain[i][j]; //On the first two els. of the diagonal
			else if(j == i) temp[i][i] = 1;
			else if (j < 2) temp[i][j] = -gain[i][j];
			else temp[i][j] = 0;
	 //To avoid copying again.
	float temp2[4][4]; //copy of t->cov; somewhere it has to be done!
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			temp2[i][j] = t->cov[i][j];
	matrixproduct(temp, temp2, t->cov);
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			printf("cov %d %d: %.40f\n", i, j, t->cov[i][j]);
	//now t->cov = P = (I -KH)P
}

void correctAll(int tracks, int blobs, kalmanTrack states[NUM_BLOBS_MAX], point centroids[NUM_BLOBS_MAX], int permutation[NUM_BLOBS_MAX]) {
	int i = 0;
	for(i = 0; i < blobs; i++) {
		if(permutation[i] < 0) continue;
		update(states+permutation[i], centroids[i]);
		states[permutation[i]].consecutiveInvisibleCount = 0;
		states[permutation[i]].totalVisibleCount++;
	}
}

int efficientKalmanCentroids(int width, int height, bitimg_t image[WIDTH*HEIGHT/8], point centroids[NUM_BLOBS_MAX]) {
	bitimg_t temp[BYTES_FOR(WIDTH)*HEIGHT];
	int numCentroids = 0;
	int k;
	for(k = 0; k < sizeof temp; k++) temp[k] = 0;
	while (!labir_extract(image, temp) && numCentroids < NUM_BLOBS_MAX) {
		int j, i;
		int x = 0, y = 0, c = 0;
		unsigned short minX = width+1;
		unsigned short minY = height+1;
		unsigned short maxX = 0;
		unsigned short maxY = 0;
		/** Identify centroid and bounding box for the blob*/
		for (j = 0; j < HEIGHT; j++) {
			for (i = 0; i < WIDTH; i++) {
				if (at(temp, i, j)) {
					c++;
					x += i; y += j;
					if(minX > i) minX = i;
					if(minY > j) minY = j;
					if(maxX < i) maxX = i;
					if(maxY < j) maxY = j;
				}
			}
		}

		centroids[numCentroids].X = x/c;
		centroids[numCentroids].Y = y/c;
		centroids[numCentroids].topX = minX;
		centroids[numCentroids].topY = minY;
		centroids[numCentroids].botX = maxX;
		centroids[numCentroids].botY = maxY;
		numCentroids++;
		for(i = 0; i < sizeof temp; i++) temp[i] = 0;
	}

	return numCentroids;
}


int kalmanCentroids(int width, int height, label_t expanded[WIDTH*HEIGHT], point res[NUM_BLOBS_MAX]) {
	unsigned int count[NUM_BLOBS_MAX];
	unsigned int x_coord[NUM_BLOBS_MAX];
	unsigned int y_coord[NUM_BLOBS_MAX];
	int i,j;
	for(i = 0; i < NUM_BLOBS_MAX; i++) {
		count[i] = 0;
		x_coord[i] = 0;
		y_coord[i] = 0;
	}
	for(j = 0; j < height; j++)
	for (i = 0; i < width; i++) {
		count[expanded[j*width+i]]++;
		x_coord[expanded[j*width+i]] += i;
		y_coord[expanded[j*width+i]] += j;
	}

	for(i = 0; i < NUM_BLOBS_MAX && count[i] > 0; i++) {
		if(x_coord[i] > 0 && y_coord[i] > 0) {
			res[i].X = (x_coord[i]/count[i]);
			res[i].Y = (y_coord[i]/count[i]);
		}
	}
	return i;
}
