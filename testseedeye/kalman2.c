
#include "utils.h"
#include "util.h"
#include "kalman2.h"
#include "labirinthlabeler.h"

#define KALMAN_OPTIMIZE 1
#define PIC_VERSION
#define MOTION_NOISE 0.1
#define MEASUREMENT_NOISE 0.1

#define VISIBLE_TRESHOLD 2
#define DELETION_TRESHOLD 2

//#define KALMAN_PERFORMANCE

static float measurementNoiseM[2][2] = {
		{0.1, 0.1},
		{0.1, 0.1},
};

static float transitionMatrix[4][4] = {
	{1, 0, 1, 0},
	{0, 1, 0, 1},
	{0, 0, 1, 0},
	{0, 0, 0, 1}
};

static float transitionMatrixT[4][4] = {
	{1, 0, 0, 0},
	{0, 1, 0, 0},
	{1, 0, 1, 0},
	{0, 1, 0, 1}
};




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
 *
 * More straightforwar implementation of the covariance matrix prediction.
 */

/** Uses aux. space O(N) and 128 operations */
static inline void unoptimizedPredictCovariance(float cov[4][4]) {
	float temp[4][4];
	matrixproduct(transitionMatrix, cov, temp);
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
/**
 * Performs a prediction
 */
void predict(kalmanTrack* t) {
	/** Update exteem of the position */
	t->posX += t->velX;
	t->posY += t->velY;
	//Speed has not to be updated in our model
	/** Update covariances */
	#ifdef KALMAN_OPTIMIZE
	optimizedPredictCovariance(t->cov);
	#else
	unoptimizedPredictCovariance(t->cov);
	#endif
	int i;
	for(i = 0; i < 4; i++) { t->cov[i][i] += MOTION_NOISE;} //Q is only diagonal. It is assumed constant.
}
/** Deletes a track */
int delete(int size, kalmanTrack states[NUM_BLOBS_MAX], int pos) {
	if(pos == size-1) return pos;
	states[pos] = states[size-1];
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
	#ifdef KALMAN_PERFORMANCE
		EE_UINT32 time = get_time_stamp();
	#endif
		predict(states+i);
	#ifdef KALMAN_PERFORMANCE
		myprintf("SinglePredict\t%d\n", elapsed_us(time, get_time_stamp()));
	#endif
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
	/** Innovation measure */
	int y[2] = { measure.X - t->posX, measure.Y - t->posY};
	//Temp = S = H x Pk x H^t + R (H is I_2). In reality S is two by two but we fill the rest with
	//zero so that we can avoid to declare another temporary matrix in the following.
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			if(i >= 2 || j >= 2) temp[i][j] = 0;
			else temp[i][j] = (t->cov[i][j] + measurementNoiseM[i][j]);
		}

	}

	float l = (temp[0][0]*temp[1][1]) - (temp[0][1]*temp[1][0]);

	float gain[4][2] = {
			{(t->cov[0][0] * temp[1][1] - t->cov[0][1]*temp[1][0])/l, (t->cov[0][1] * temp[0][0] - t->cov[0][0]* temp[0][1])/l},
			{(t->cov[1][0] * temp[1][1] - t->cov[1][1]*temp[1][0])/l, (t->cov[1][1] * temp[0][0] - t->cov[1][0]*temp[0][1])/l},
			{(t->cov[2][0] * temp[1][1] - t->cov[2][1]*temp[1][0])/l, (t->cov[2][1] * temp[0][0] - t->cov[2][0]*temp[0][1])/l},
			{(t->cov[3][0] * temp[1][1] - t->cov[3][1]*temp[1][0])/l, (t->cov[3][1] * temp[0][0] - t->cov[3][0]*temp[0][1])/l}
	};

	//Use the gain to weight the innovation in the update phase.
	t->posX += gain[0][0]*y[0] + gain[0][1]*y[1];
	int Y_update = (int)(gain[1][0]*y[0] + gain[1][1]*y[1]);
	//myprintf("Y upd %.5f \n", gain[1][0]*y[0] + gain[1][1]*y[1]);
	t->posY += Y_update;
	t->velX += gain[2][0]*y[0] + gain[2][1]*y[1];
	t->velY += gain[3][0]*y[0] + gain[3][1]*y[1];
	//Update the covariance matrix.

	//temp will now become (I-KH) ->  Identity - gain*I_2
	//TODO: check if there's need to optimize or if the compiler will take car.e
	 for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			if(j == i && i < 2) temp[i][j] = 1 - gain[i][j]; //On the first two elements of the diagonal
			else if(j == i) temp[i][i] = 1;
			else if (j < 2) temp[i][j] = -gain[i][j];
			else temp[i][j] = 0;

	float temp2[4][4];
	//copy old t->cov.
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			temp2[i][j] = t->cov[i][j];
	matrixproduct(temp, temp2, t->cov);
	//now t->cov = P = (I -KH)P
}

void correctAll(int tracks, int blobs, kalmanTrack states[NUM_BLOBS_MAX], point centroids[NUM_BLOBS_MAX], int permutation[NUM_BLOBS_MAX]) {
	int i = 0;
	for(i = 0; i < blobs; i++) {
		if(permutation[i] < 0) continue;
#ifdef KALMAN_PERFORMANCE
		EE_UINT32 time = get_time_stamp();
#endif
		update(states+permutation[i], centroids[i]);
#ifdef KALMAN_PERFORMANCE
		myprintf("Update\t%d\n", elapsed_us(time, get_time_stamp()));
#endif
		states[permutation[i]].consecutiveInvisibleCount = 0;
		states[permutation[i]].totalVisibleCount++;
	}
}
/**
 * Interface for the labeling.
 */
int efficientKalmanCentroids(int width, int height, bitimg_t image[WIDTH*HEIGHT/8], point centroids[NUM_BLOBS_MAX]) {
	//bitimg_t temp[BYTES_FOR(WIDTH)*HEIGHT];
	int numCentroids = 0;
	int k;
	int cx,cy,tx,ty,bx,by;
	//for(k = 0; k < sizeof temp; k++) temp[k] = 0;
	labir_init2();
	while (!labir_extract2(image, &cx, &cy, &tx, &ty, &bx, &by) && numCentroids < NUM_BLOBS_MAX) {
		centroids[numCentroids].X = cx;
		centroids[numCentroids].Y = cy;
		centroids[numCentroids].topX = tx;
		centroids[numCentroids].topY = ty;
		centroids[numCentroids].botX = bx;
		centroids[numCentroids].botY = by;
		numCentroids++;
	}

	return numCentroids;
}
/** Calculates centroids from already labeled image */
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

//Test 2d kalman filter
int test2dKalman(int num_iter){
	int i,j;
		point p;
		p.X = 0;
		p.Y = 0;
		kalmanTrack t;
		for(i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++)
				t.cov[i][j] = 0;
		}
		t.posX = 0;
		t.posY = 0;

		t.velX = 0;
		t.velY = 0;
		for(i = 0; i < num_iter; i++) {
			int r1 = rand() % 2;
			p.X+=(r1) ? 2 : 1;
			r1 = rand()%2;
			p.Y+=(r1) ? 4 : 2;
			predict(&t);

			int k;
			for(j = 0; j < 4; j++) {
				for(k = 0; k < 4; k++) {
					printf("%.2f \t", t.cov[j][k]);
				}
				printf("\n");
			}
			update(&t, p);

		}


		return 0;
}

