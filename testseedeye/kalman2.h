/*
 * kalman2.h
 *
 *  Created on: 08/set/2014
 *      Author: alessandro
 */

#include "utils.h"
#include "labirinthlabeler.h"
#ifndef KALMAN2_H_
#define KALMAN2_H_



typedef struct {
	short X;
	short Y;
	short topX;
	short topY;
	short botX;
	short botY;
} point;


static float measurementNoiseM[4][4] = {
		{0.1, 0.1, 1, 1},
		{0.1, 0.2, 2, 2},
		{0.3, 0.1, 3, 3},
		{0.1, 0.9, 4, 1}
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

typedef struct {
	/** State */
	short posX, posY;
	float velX, velY;
	/** Covariance matrix */
	float cov[4][4];
	short id;
	short age;
	short totalVisibleCount;
	short consecutiveInvisibleCount;

} kalmanTrack;

/*
 * kalman2.c
 *
 *  Created on: 08/set/2014
 *      Author: alessandro
 */
#include "utils.h"
#include "kalman2.h"

#define KALMAN_OPTIMIZE 1
#define MOTION_NOISE 0.1
#define MEASUREMENT_NOISE 0.1

#define NUM_BLOBS_MAX 64

#define VISIBLE_TRESHOLD 2
#define DELETION_TRESHOLD 2

/** Call before start using Kalman Filter. Used to manage the identifiers of the tracks */
void initialize();
/* Assigns an id to a new track, possibly reusing ids of previously deleted tracks.*/
int assign();
/**
 * Makes a new prediction for the track t
 */

/** Uses aux. space O(N) and 128 operations
void unoptimizedPredictCovariance(float cov[4][4]);
void optimizedPredictCovariance(float cov[4][4]);
*/
void predict(kalmanTrack* t);
/** Deletes a track */
int delete(int size, kalmanTrack states[NUM_BLOBS_MAX], int pos);
/**
 * Eliminates unused tracks and predicts for the remaining ones.
 */
int predictAll(int size, kalmanTrack states[NUM_BLOBS_MAX]);
/**
 * Performs a correction of the track based on the measure
 * */

void update(kalmanTrack* t, point measure);
void correctAll(int tracks, int blobs, kalmanTrack states[NUM_BLOBS_MAX], point centroids[NUM_BLOBS_MAX], int permutation[NUM_BLOBS_MAX]);

int efficientKalmanCentroids(int width, int height, bitimg_t image[WIDTH*HEIGHT/8], point centroids[NUM_BLOBS_MAX]);

int kalmanCentroids(int width, int height, label_t expanded[WIDTH*HEIGHT], point res[NUM_BLOBS_MAX]);

#endif /* KALMAN2_H_ */
