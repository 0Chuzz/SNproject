/*
 * kalman.h
 *
 *  Created on: 05/set/2014
 *      Author: alessandro
 */
#include "utils.h"
#ifndef KALMAN_H_
#define KALMAN_H_
/** State of an object in kalman filter


typedef struct {
	short id;
	point_t prev_measure;
	short posX;
	short posY;
	float velX;
	float velY;
} kalmanState_t;


typedef struct {
	short X;
	short Y;
} point_t;

int assign();

void computeCostMatrix(kalmanState_t prediction[], size_t numPred, point_t measures[], size_t numMeas, float costMatrix[numPred][numMeas]);

void hungarianMethodAssignment(float costMatrix[][256], int rows, int cols, //in parameters
								int permutation[], short unassignedRows[], short* unassignedRowNum,
								short unassignedCols[], short *unassignedColsNum); //out parameters

float euclideanDistance(int x1, int y1, int x2, int y2);

void predict(kalmanState_t lastExt[], float Q, float R, int *worthTracking);

void correct(kalmanState_t prediction[], int permutation[], point_t measures[], int track, int blob);

void track(kalmanState_t prediction[], point_t measures[], int permutation[], int n);

int kalmanCentroids(label_t expanded[], int w, int h, point_t res[]);*/
#endif /* KALMAN_H_ */
