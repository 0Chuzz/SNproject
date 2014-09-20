/*
 * minassign.c
 *
 *  Created on: 07/set/2014
 *      Author: alessandro
 */
#include "minassign.h"
#include "utils.h"
#include "approximatedKalman.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct {
	short start;
	short end;
	float cost;
} edge_t;

typedef struct {
	short start;
	short end;
	int cost;
} intEdge_t;

int edgecmpr(const void * e1, const void*e2) {
	edge_t* first = (edge_t*) e1;
	edge_t* second = (edge_t*) e2;
	if (first->cost < second->cost) return -1;
	else if (first->cost > second->cost) return 1;
	return 0;
}

int intEdgecmpr(const void *e1, const void*e2) {
	intEdge_t * first = (intEdge_t*)e1;
	intEdge_t * second = (intEdge_t*)e2;
	if (first->cost < second->cost) return -1;
	else if (first->cost > second->cost) return 1;
	return 0;
}

static inline int distance(int x1, int y1, int x2, int y2) {
	return (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
}

static inline float euclideanDistance(int x1, int y1, int x2, int y2) {
	return  (float) sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

int fastFindAssignment(int rows, int cols, approxKalmanTrack_t predictions[NUM_BLOBS_MAX], point blobs[NUM_BLOBS_MAX], int unassignmentCost,
						int permutation[NUM_BLOBS_MAX], short unassignedBlobs[NUM_BLOBS_MAX], short *unassignedBlobsNum) {
	int unassignmentCost2 = unassignmentCost*unassignmentCost;
	int i, j, e = 0;
	intEdge_t distances[rows*cols];
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			distances[e].cost = distance(predictions[i].posX, predictions[i].posY, blobs[j].X, blobs[j].Y);
			distances[e].start = i;
			distances[e].end = j;
			e++;
		}
	}
	qsort(distances, rows*cols, sizeof(intEdge_t), intEdgecmpr);

		//Assignment cicle
	char assignedR[rows], assignedC[cols];
	for(i = 0; i < rows; i++) assignedR[i] = assignedC[i] = 0;

	for(i = 0; i < rows*cols; i++) {
		if(distances[i].cost > unassignmentCost2) break;
		if (assignedR[distances[i].start] == 0 && assignedC[distances[i].end] == 0) {
			permutation[distances[i].end] = distances[i].start;
			assignedR[distances[i].start] = 1;
			assignedC[distances[i].end] = 1;
		}
	}
	e =0;
	for(i = 0; i < cols; i++) {
		if(assignedC[i] == 0) {
			unassignedBlobs[e] = i;
			e++;
		}
	}
	(*unassignedBlobsNum) = e;

	return 0;
}

int findAssignment(int rows, int cols, kalmanTrack predictions[NUM_BLOBS_MAX], point blobs[NUM_BLOBS_MAX], float unassignmentCost,
					int permutation[NUM_BLOBS_MAX], short unassignedBlobs[NUM_BLOBS_MAX], short *unassignedBlobsNum) {
	int i, j, e=0;
	edge_t distances[rows*cols];

	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			distances[e].cost = euclideanDistance(predictions[i].posX, predictions[i].posY, blobs[j].X, blobs[j].Y);
			distances[e].start = i;
			distances[e].end = j;
			e++;
		}
	}
	qsort(distances, rows*cols, sizeof(edge_t), edgecmpr);

	//Assignment cicle
	char assignedR[rows], assignedC[cols];
	for(i = 0; i < rows; i++) assignedR[i] = assignedC[i] = 0;

	for(i = 0; i < rows*cols; i++) {
		if(distances[i].cost > unassignmentCost) break;
		if (assignedR[distances[i].start] == 0 && assignedC[distances[i].end] == 0) {
			permutation[distances[i].end] = distances[i].start;
			assignedR[distances[i].start] = 1;
			assignedC[distances[i].end] = 1;
		}
	}
	e =0;
	for(i = 0; i < cols; i++) {
		if(assignedC[i] == 0) {
			unassignedBlobs[e] = i;
			e++;
		}
	}
	(*unassignedBlobsNum) = e;

	return 0;
}
