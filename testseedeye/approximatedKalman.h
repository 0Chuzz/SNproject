/*
 * approximatedKalman.h
 *
 *  Created on: 16/set/2014
 *      Author: Alessandro
 */

#ifndef APPROXIMATEDKALMAN_H_
#define APPROXIMATEDKALMAN_H_

#include "kalman2.h"

#define MOTION_NOISE 0.1
#define MEASUREMENT_NOISE 0.1

#define NUM_BLOBS_MAX 64

#define VISIBLE_TRESHOLD 2
#define DELETION_TRESHOLD 2

#define INITIAL_AUTOCOV 100

/** Value used to define the precision. i.e. UNIT_VALUE = 100 means that the fixed point
 * precision will be of 10^-2.
 * Increase for further precision, decrease for less. (1 => bare integer)
 */
#define UNIT_VALUE 100

typedef struct {
	/** State */
	short posX, posY;
	int velX, velY;
	/** Covariance matrix */
	int cov[4][4];
	short id;
	short age;
	short totalVisibleCount;
	short consecutiveInvisibleCount;

} approxKalmanTrack_t;


static float approxMeasurementNoiseM[4][4] = {
		{0.1, 0.1, 1, 1},
		{0.1, 0.2, 2, 2},
		{0.3, 0.1, 3, 3},
		{0.1, 0.9, 4, 1}
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


void approximate_predict(approxKalmanTrack_t* t);
/** Deletes a track */
int approximate_delete(int size, approxKalmanTrack_t states[NUM_BLOBS_MAX], int pos);
/**
 * Eliminates unused tracks and predicts for the remaining ones.
 */

int approximate_predictAll(int size, approxKalmanTrack_t states[NUM_BLOBS_MAX]);

/**
 * Performs a correction of the track based on the measure
 * */
void approximate_update(approxKalmanTrack_t* t, point measure);
/** Performs a correction on all the tracks using the detected blobs. Assignment
 * is made through the permutaation vector
 */
void approximate_correctAll(int tracks, int blobs, approxKalmanTrack_t states[NUM_BLOBS_MAX], point centroids[NUM_BLOBS_MAX], int permutation[NUM_BLOBS_MAX]);

/** Test kalman 2d for precision */

int approximate_test2dKalman(int num_iter);


#endif /* APPROXIMATEDKALMAN_H_ */
