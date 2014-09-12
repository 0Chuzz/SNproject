/*
 * kalman.c
 *
 *  Created on: 05/set/2014
 *      Author: alessandro
 */

#include "utils.h"
#include "kalman.h"
#include <math.h>
#include <stdio.h>


#define VISIBLE_TRESHOLD 2
#define DELETION_TRESHOLD 2

unsigned short freePos[256];
unsigned short freedPos = 0;
unsigned short lastPosEverUsed = 0;
/*
void delete(kalmanState_t state[], int *count, int pos) {
	int i;
	freePos[freedPos] = state[pos].id;
	freedPos++;
	for(i = pos; i+1 < *count; i++) {
		state[i] = state[i+1];
	}
	(*count)--;
}

int assign() {
	if (freedPos > 0) { //recycle.
		freedPos--;
		printf("Assigning a new value: %d, %d\n", freePos[freedPos], freedPos);
		return freePos[freedPos];
	}
	return lastPosEverUsed++;
}


void predict(kalmanState_t lastExt[], float Q, float R, int *worthTracking) {
	int i;

	posXCov += velXcov+Q;
	posYCov += velYcov+Q;
	velXcov += accXcov+Q;
	velYcov += accXcov+Q;

	for(i = 0; i < *worthTracking; i++) {
		if(lastExt[i].consecutiveInvisibleCount++ >= DELETION_TRESHOLD) {
			delete(lastExt, worthTracking, i);
			i--;
		}
		lastExt[i].age++;
		lastExt[i].posX += lastExt[i].velX;
		lastExt[i].velX += lastExt[i].accX;
		lastExt[i].posY += lastExt[i].velY;
		lastExt[i].velY += lastExt[i].accY;
	}
}


void predict(kalmanState_t state, point_t measure) {

}


void correct(kalmanState_t prediction[], int permutation[], point_t measures[], int tracks, int blobs) {
	int i,j;
	for(i = 1; i < blobs; i++) {
		//Find right track for the blob
		for(j = 0; j < tracks; j++){
			if(prediction[j].id == permutation[i]) {
				prediction[j].posX = prediction[j].posX + prediction[j].gainX * (measures[i].X - prediction[j].posX);
				prediction[j].posY = prediction[j].posY + prediction[j].gainY * (measures[i].Y - prediction[j].posY);
				prediction[j].velX = (1- prediction[j].gainX) * prediction[j].velX;
				prediction[j].velY = (1-prediction[j].gainY) * prediction[j].velY;
				prediction[j].consecutiveInvisibleCount = 0;
				prediction[j].totalVisibleCount++;
				continue;
			}
		}

	}
}

float euclideanDistance(int x1, int y1, int x2, int y2) {
	return  (float) sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

int kalmanCentroids(label_t expanded[], int w, int h, point_t res[]) {
	unsigned int count[256];
	unsigned int x_coord[256];
	unsigned int y_coord[256];
	int i,j;
	for(i = 0; i < 256; i++) {
		count[i] = 0;
		x_coord[i] = 0;
		y_coord[i] = 0;
	}
	for(j = 0; j < h; j++)
	for (i = 0; i < w; i++) {
		count[expanded[j*w+i]]++;
		x_coord[expanded[j*w+i]] += i;
		y_coord[expanded[j*w+i]] += j;
	}


	for(i = 0; i < 256 && count[i] > 0; i++) {
		if(x_coord[i] > 0 && y_coord[i] > 0) {
			res[i].X = (x_coord[i]/count[i]);
			res[i].Y = (y_coord[i]/count[i]);
		}
	}
	return i;
}
//Notice, you have to take into account that the number of predictions and measures are not always the same!

void computeCostMatrix(kalmanState_t prediction[], size_t numPred, point_t measures[], size_t numMeas, float costMatrix[numPred][numMeas]) {
	int i,j;
	for(i = 0; i < numPred; i++) {
		for (j = 0; j < numMeas; j++) {
			costMatrix[i][j] = euclideanDistance(prediction[i].posX, prediction[i].posY, measures[j].X, measures[j].Y);
		}
	}
}

void hungarianMethodAssignment(float costMatrix[][256], int rows, int cols,
		int permutation[], short unassignedRows[], short* unassignedRowNum,
		short unassignedCols[], short *unassignedColsNum) {
	int i, j;

	for(i = 0; i < rows; i++) {
		float min = costMatrix[i][0];
		for(j = 1; j < cols; j++) {
			if(costMatrix[i][j] < min) {
				min = costMatrix[i][j]; // subtract minimum
			}
		}
		if(min < 50) for(j = 0; j < cols; j++) costMatrix[i][j] -= min; //subtract minimum to each row
	}
	int minAssignment = (rows < cols) ? rows : cols;
	unsigned char assignments[minAssignment][2];
	int assignedCols = 0;

	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			if (costMatrix[i][j] == 0) {
				//check if it is the only starred in assignments
				int k;
				int found = 0;
				for(k = 0; k < assignedCols; k++) {
					if (assignments[k][0] == i || assignments[k][1] == j) {
						found = 1;
						break;
					}
				}
				if(!found) { //Star the zero.
					assignments[assignedCols][0] = i;
					assignments[assignedCols][1] = j;
					permutation[j] = i;
					assignedCols++;
				}
			}
		}
	}
	if(assignedCols == minAssignment) return;
	//Manage unassignment case
	//Find all unassigned columns.
	short unassignedC = 0;

	for (i = 0; i < cols; i++) {
		short found = 0;
		for(j = 0; j < assignedCols; j++) {
			if  (assignments[j][1] == i) { found = 1; continue;}
		}
		if(!found) unassignedCols[unassignedC++] = i;
		printf("Searched for %d\n", i);
	}

	//Find all unassigned rows.
	short unassignedR = 0;

	for(i = 0; i < rows; i++) {
		short found = 0;
		for (j = 0; j < assignedCols; j++) {
			if(assignments[j][0] == i) {found = 1; continue;}
		}
		if(!found)
			unassignedRows[unassignedR++] = i;
	}

	*unassignedRowNum = unassignedR;
	*unassignedColsNum = unassignedC;
	return;
}


void track(kalmanState_t prediction[], point_t measures[], int permutation[], int n) {
	int i,j;
	short assigned[256];
	short unassigned[256];
	int un = 0;
	int treshold = 100;
	permutation[0] = 0;
	for (i = 1; i < n; i++) {
		float tmp, minDist = 320*240;
		int minMap = i;
		for(j = 1; j < n; j++) {
			if(assigned[j]) continue;
			tmp = euclideanDistance(prediction[i].posX, prediction[i].posY, measures[j].X, measures[j].Y);
			if (tmp < minDist) {
				minMap = j;
				minDist = tmp;
			}
		}
		if(minDist < treshold) {
			permutation[i] = minMap;
			assigned[minMap] = 1;
		} else {
			unassigned[un++] = i;
		}
	}
	printf("How many unassigned? %d\n", un);
	int startAt = 1;
	for (i = 0; i < un; i++) {
		for(j = startAt; j < 256; j++) {
			if(assigned[j] != 1) {
				printf("Now assigning values to %d %d\n", unassigned[i], j);
				permutation[unassigned[i]] = j;
				startAt = j+1;
				assigned[i] = 1;
				break;
			}
		}
	}
	printf("FInished calculating permutations:\n");
	for(i = 1; i < n; i++) {
		printf("%d -> %d \n", i, permutation[i]);
	}

}
*/

