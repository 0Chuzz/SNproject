/*
 * kalman1.h
 *
 *  Created on: 09/set/2014
 *      Author: alessandro
 */
#include "kalman2.h"
#ifndef KALMAN1_H_
#define KALMAN1_H_

#define MOTION_NOISE_INT 5;

typedef struct {
	short X;
	short Y;
	short cov[2][2];
} shortKalmanTrack;

void initialize1();

int assig1n();


void predict1(shortKalmanTrack* t) {
	//Update the estimate covariance. Nothing happens in this step to the position.
	int i;
	for(i = 0; i < 2; i++) {
		//for(j=0; j < 2; j++) cov[i][j] += MOTION_NOISE_INT;
	}

}

size_t delete1(size_t size, kalmanTrack states[size], int pos);

size_t predictAll1(size_t size, kalmanTrack states[size]);

void update1(kalmanTrack* t, point measure);

void correctAll1(size_t tracks, size_t blobs, kalmanTrack states[tracks], point centroids[blobs], int permutation[blobs]);


#endif /* KALMAN1_H_ */
