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

#define INITIAL_AUTOCOV 1000

typedef struct {
	/** Centroid */
	short X;
	short Y;
	/** BBox */
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
	/** Err. Covariance matrix */
	float cov[4][4];

	short id;

	short age;

	short totalVisibleCount;

	short consecutiveInvisibleCount;

} kalmanTrack;


/** Call before start using Kalman Filter. Used to manage the identifiers of the tracks */
void initialize();
/* Assigns an id to a new track, possibly reusing ids of previously deleted tracks.*/
int assign();
/**
 * Makes a new prediction for the track t
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
/** Performs a correction on all the tracks using the detected blobs. Assignment
 * is made through the permutaation vector
 */
void correctAll(int tracks, int blobs, kalmanTrack states[NUM_BLOBS_MAX], point centroids[NUM_BLOBS_MAX], int permutation[NUM_BLOBS_MAX]);

/**
 * Memory efficient centroids calculation procedure. Also calculates bounding box.
 */
int efficientKalmanCentroids(int width, int height, bitimg_t image[WIDTH*HEIGHT/8], point centroids[NUM_BLOBS_MAX]);

/**
 *  Time efficient (but not memory) centroids calculation procedure. Also calculates bounding box.
 */
int kalmanCentroids(int width, int height, label_t expanded[WIDTH*HEIGHT], point res[NUM_BLOBS_MAX]);

/** Test kalman 2d for precision */

int test2dKalman(int num_iter);
#endif /* KALMAN2_H_ */
