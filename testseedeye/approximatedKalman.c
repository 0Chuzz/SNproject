/*
 * approximatedKalman.c
 *
 *  Created on: 16/set/2014
 *      Author: Alessandro
 */
#include "approximatedKalman.h"
#include "utils.h"
#include "util.h"
#include "labirinthlabeler.h"
#include <stdint.h>

#define KALMAN_OPTIMIZE 1
#define PIC_VERSION

#define VISIBLE_TRESHOLD 2
#define DELETION_TRESHOLD 2
#define HALF_UNIT_VALUE UNIT_VALUE/2
#define APPROX_KALMAN_PERFORMANCE

static float approxMeasurementNoiseM[4][4] = {
		{0.1*UNIT_VALUE, 0.1*UNIT_VALUE},
		{0.1*UNIT_VALUE, 0.1*UNIT_VALUE}
};

static int approxTransitionMatrix[4][4] = {
	{1*UNIT_VALUE, 0, 1*UNIT_VALUE, 0},
	{0, 1*UNIT_VALUE, 0, 1*UNIT_VALUE},
	{0, 0, 1*UNIT_VALUE, 0},
	{0, 0, 0, 1*UNIT_VALUE}
};

static int approxTransitionMatrixT[4][4] = {
	{1*UNIT_VALUE, 0, 0, 0},
	{0, 1*UNIT_VALUE, 0, 0},
	{1*UNIT_VALUE, 0, 1*UNIT_VALUE, 0},
	{0, 1*UNIT_VALUE, 0, 1*UNIT_VALUE}
};


static inline void matrixproduct(int m1[4][4], int m2[4][4], int result[4][4]) {
	int i,j,k;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			result[i][j] = 0;
			for(k = 0; k < 4; k++)
				result[i][j] += (result[i][k] * result[k][j])/UNIT_VALUE;
		}
	}
}


/**
 * Makes a new prediction for the track t
 */

/** Uses aux. space O(N) and 128 operations */
static inline void unoptimizedPredictCovariance(int cov[4][4]) {
	int temp[4][4];
	matrixproduct( approxTransitionMatrix, cov, temp);
	matrixproduct(cov, approxTransitionMatrixT, cov);
}
/** Uses aux. space O(N) and 32 operations */
static inline void optimizedPredictCovariance(int cov[4][4]) {
	int temp[4][4];
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
void approximate_predict(approxKalmanTrack_t* t) {
	/** Update exteem of the position */
	t->posX += (t->velX + HALF_UNIT_VALUE)/UNIT_VALUE;
	t->posY += (t->velY + HALF_UNIT_VALUE)/UNIT_VALUE;
	//Speed has not to be updated in our model
	/** Update covariances */
	#ifdef KALMAN_OPTIMIZE
	optimizedPredictCovariance(t->cov);
	#else
	unoptimizedPredictCovariance(t->cov);
	#endif
	int i;
	for(i = 0; i < 4; i++) { t->cov[i][i] += MOTION_NOISE*UNIT_VALUE;} //Q is only diagonal.
	//P = F*P*F^T + Q
}

int approximate_predictAll(int size, approxKalmanTrack_t states[NUM_BLOBS_MAX]) {
	int i;
	for(i = 0; i < size; i++) {
		if(states[i].consecutiveInvisibleCount++ >= DELETION_TRESHOLD) {
			size = approximate_delete(size, states, i); //delete is linear, could be better. But we don't care.
			i--;
		}
	#ifdef APPROX_KALMAN_PERFORMANCE
			EE_UINT32 time = get_time_stamp();
	#endif
			approximate_predict(states+i);
	#ifdef APPROX_KALMAN_PERFORMANCE
			myprintf("Update\t%d\n", elapsed_us(time, get_time_stamp()));
	#endif
		states[i].age++;
	}
	return size;
}

/** Deletes a track */
int approximate_delete(int size, approxKalmanTrack_t states[NUM_BLOBS_MAX], int pos) {
	if(pos == size -1) return pos;
	states[pos] = states[size-1];
	return size -1;
}

/**
 * Performs a correction of the track based on the measure
 * */

void approximate_update(approxKalmanTrack_t* t, point measure) {
	int temp[4][4];
	int i, j;
	/** Vect is Y */

	int y[2] = { measure.X - t->posX, measure.Y - t->posY};
	//Temp = S = H x Pk x H^t + R (H is I_2)
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			if(i >= 2 || j >= 2) temp[i][j] = 0;
			else temp[i][j] = (t->cov[i][j] + approxMeasurementNoiseM[i][j]);
		}
	}
	#ifdef DEBUG
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++) {
				myprintf("TEMP: %d %d %d\n", i, j, temp[i][j]);
			}
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++) {
				myprintf("COV: %d %d %d\n", i, j, t->cov[i][j]);
			}
	#endif
	int l1 = HALF_UNIT_VALUE + (((temp[0][0]) * (temp[1][1])) - ((temp[0][1])*(temp[1][0])))/UNIT_VALUE;
	int32_t gain[4][2] = {
			{(t->cov[0][0] * temp[1][1] - t->cov[0][1]*temp[1][0])/l1, (t->cov[0][1] * temp[0][0] - t->cov[0][0]* temp[0][1])/l1},
			{(t->cov[1][0] * temp[1][1] - t->cov[1][1]*temp[1][0])/l1, (t->cov[1][1] * temp[0][0] - t->cov[1][0]*temp[0][1])/l1},
			{(t->cov[2][0] * temp[1][1] - t->cov[2][1]*temp[1][0])/l1, (t->cov[2][1] * temp[0][0] - t->cov[2][0]*temp[0][1])/l1},
			{(t->cov[3][0] * temp[1][1] - t->cov[3][1]*temp[1][0])/l1, (t->cov[3][1] * temp[0][0] - t->cov[3][0]*temp[0][1])/l1}
	};
	#ifdef DEBUG
		for(i = 0; i < 4; i++)
			for(j = 0; j < 2; j++)
				myprintf("%d %d --> %d\n", i, j, gain[i][j]);
		//myprintf("l value is %5.f\n", l);
	#endif


	t->posX += ((gain[0][0]*y[0]) + (gain[0][1]*y[1]) + HALF_UNIT_VALUE)/UNIT_VALUE;
	int Y_update = (int)((gain[1][0]*y[0]) + (gain[1][1]*y[1]) + HALF_UNIT_VALUE)/UNIT_VALUE;
	t->posY += Y_update;
	t->velX += (gain[2][0]*y[0] + gain[2][1]*y[1]);
	t->velY += (gain[3][0]*y[0] + gain[3][1]*y[1]);

	#ifdef DEBUG
	myprintf("new values are \n");
	myprintf("%d,%d and %d,%d", t->posX, t->posY, t->velX, t->velY);
	#endif

	//Update the covariance matrix.

	//temp is now (I-KH) ->  Identity - gain*I_2
	//TODO: check if there's need to optimize or if the compiler will take car.e
	 for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			if(j == i && i < 2) temp[i][j] = UNIT_VALUE - gain[i][j]; //On the first two els. of the diagonal
			else if(j == i) temp[i][i] = UNIT_VALUE;
			else if (j < 2) temp[i][j] = -gain[i][j];
			else temp[i][j] = 0;
	 //To avoid copying again.
	int temp2[4][4]; //copy of t->cov; somewhere it has to be done!
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			temp2[i][j] = t->cov[i][j];
	matrixproduct(temp, temp2, t->cov);
}

void approximate_correctAll(int tracks, int blobs, approxKalmanTrack_t states[NUM_BLOBS_MAX], point centroids[NUM_BLOBS_MAX], int permutation[NUM_BLOBS_MAX]) {
	int i = 0;
	for(i = 0; i < blobs; i++) {
		if(permutation[i] < 0) continue;
#ifdef APPROX_KALMAN_PERFORMANCE
		EE_UINT32 time = get_time_stamp();
#endif
		approximate_update(states+permutation[i], centroids[i]);
#ifdef APPROX_KALMAN_PERFORMANCE
		myprintf("SinglePredict\t%d\n", elapsed_us(time, get_time_stamp()));
#endif
		states[permutation[i]].consecutiveInvisibleCount = 0;
		states[permutation[i]].totalVisibleCount++;
	}
}

//Test 2d kalman filter
int approximate_test2dKalman(int num_iter){
	int i,j;
		point p;
		p.X = 0;
		p.Y = 0;
		approxKalmanTrack_t t;
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
			printf("Prediction says:\n");
			printf("Pos: (%d, %d)\n", t.posX, t.posY);
			printf("Speed: (%.2f, %.2f)\n", t.velX, t.velY);
			int k;
			for(j = 0; j < 4; j++) {
				for(k = 0; k < 4; k++) {
					printf("%.2f \t", t.cov[j][k]);
				}
				printf("\n");
			}
			update(&t, p);
			printf("RealPosition is %d, %d\n", p.X, p.Y);
		}


		return 0;
}



