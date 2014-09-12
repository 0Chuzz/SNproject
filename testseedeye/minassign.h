/*
 * minassign.h
 *
 *  Created on: 07/set/2014
 *      Author: alessandro
 */

#include <stddef.h>
#include "kalman2.h"
#include "utils.h"
#ifndef MINASSIGN_H_
#define MINASSIGN_H_
/*
int findAssignment(size_t rows, size_t cols, kalmanState_t predictions[rows], point_t blobs[cols], float unassignmentCost,
					int permutation[cols], short unassignedBlobs[cols], short *unassignedBlobsNum);
int minAssignHeuristic(size_t rows, size_t cols, float costMatrix[rows][cols], int permutation[], short unassignedRows[rows], short *unassignedRowNum, short unassignedCols[cols], short * unassignedColNum);
*/
int findAssignment(int rows, int cols, kalmanTrack predictions[NUM_BLOBS_MAX], point blobs[NUM_BLOBS_MAX], float unassignmentCost,
					int permutation[NUM_BLOBS_MAX], short unassignedBlobs[NUM_BLOBS_MAX], short *unassignedBlobsNum);
#endif /* MINASSIGN_H_ */
