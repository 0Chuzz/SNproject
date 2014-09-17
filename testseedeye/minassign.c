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
	int start;
	int end;
	float cost;
} edge_t;

typedef struct {
	int start;
	int end;
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

int distance(int x1, int y1, int x2, int y2) {
	return (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
}

float euclideanDistance(int x1, int y1, int x2, int y2) {
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
	short assignedR[rows], assignedC[cols];
	for(i = 0; i < rows; i++) assignedR[i] = 0;
	for(i = 0; i < cols; i++) assignedC[i] = 0;

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
	short assignedR[rows], assignedC[cols];
	for(i = 0; i < rows; i++) assignedR[i] = 0;
	for(i = 0; i < cols; i++) assignedC[i] = 0;

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

/*
int minAssignHeuristic(size_t rows, size_t cols, float costMatrix[rows][cols], int permutation[], short unassignedRows[rows], short *unassignedRowNum, short unassignedCols[cols], short * unassignedColNum) {
	edge_t edges[rows*cols];
	unsigned short i, j;
	unsigned short assignedR[rows];
	unsigned short assignedC[cols];
	int min = (rows < cols) ? rows : cols;
	int remainder = (rows < cols) ? cols - rows : rows -cols;
	unsigned short *coll = (rows < cols) ? assignedC : assignedR;
	assignedR[0] = 1;
	assignedC[0] = 1;
	permutation[0] = 0;
	for(i = 1; i < min; i++) {
		assignedR[i] = assignedC[i] = 0;
	}
	for(;i<min+remainder; i++) {
		coll[i] = 0;
	}
	int e = 0;
	for(i = 0; i < rows; i++)
		for(j = 0; j < cols; j++) {
			edges[e].cost = costMatrix[i][j];
			edges[e].start = i;
			edges[e].end = j;
			e++;
		}
	//apply sorting
	qsort(edges, rows*cols, sizeof(edge_t), edgecmpr);
	int worstMatches = 0;
	int cost = 0;
	for(i = 0; i < min+worstMatches; i++) {
		if(edges[i].cost > UNASSIGNMENT_COST) break;
		if (assignedR[edges[i].start] == 0 && assignedC[edges[i].end] == 0) {
			assignedR[edges[i].start] = 1;
			assignedC[edges[i].end] = 1;
			permutation[edges[i].end] = edges[i].start;
			printf("Assignment!\n");
		} else worstMatches++;
	}
	j= 0;
	for(i = 0; i < rows; i++) {
		if(!assignedR[i])
			unassignedRows[j++] = i;
	}
	*unassignedRowNum = (short) j;
	j = 0;
	for(i = 0; i < cols; i++) {
		if(!assignedC[i])
			unassignedCols[j++] = i;
	}
	*unassignedColNum = (short) j;
	printf("Not able to assign %d rows and %d cols \n", *unassignedRowNum, *unassignedColNum);
	return cost;
}

void resetCovering() {
	int i = 0;
	for(; i < NUM_BLOBS_MAX; i++) {
		rowCover[i] = colCover[i] = 0;
	}
}
/*
int runminassign(float costMatrix[][NUM_BLOBS_MAX]) {
	resetCovering();
	curStar = 0;
	short done = 0;
	while(!done) {
		switch (step) {
		case 1: one(costMatrix); break;
		case 2: two(costMatrix);break;
		case 3: three(costMatrix);break;
		case 4: four(costMatrix);break;
		case 5: five(costMatrix);break;
		case 6: six(costMatrix);break;
		case 7: seven(costMatrix); done = 1; break;
		}
	}
	return 1;
}*/
/*
void one(float costMatrix[][NUM_BLOBS_MAX], int rows, int cols) {
	int i, j;
	for(i = 0; i < rows; i++) {
		float min = costMatrix[i][0];
		for(j = 1; j < cols; j++) {
			if(costMatrix[i][j] < min) {
				min = costMatrix[i][j]; // subtract minimum
			}
		}
		for(j = 0; j < cols; j++) costMatrix[i][j] -= min; //subtract minimum to each row
	}
	step = 2;
}

void two(float costMatrix[][NUM_BLOBS_MAX], int rows, int cols) {
	int i, c;

	for (i = 0; i < rows; i++) {
		for (c = 0; c < cols; c++) {
			if (costMatrix[i][c] == 0 && rowCover[i] == 0 && colCover[c] == 0) {
				//We can star the array
				rowCover[i] = colCover[c] = 1;
				//star the element (check most efficient way)
				stars[curStar][0] = i;
				stars[curStar][1] = c;
				curStar++;
			}
		}
	}
	resetCovering();
	step = 3;
}

void three(float costMatrix[][NUM_BLOBS_MAX], int rows, int cols) {
	int i;
	for(i = 0; i < curStar; i++) {
		colCover[stars[i][1]] = 1;
	}

	if (curStar >= rows || curStar >= cols) {
		step = 7; return;
	}
	step = 4;
}

void find_a_zero(short * rowStart, short * colStart, int rowN, int colN, float costMatrix[][NUM_BLOBS_MAX]) {
	short r = 0;
	short c = 0;
	short d = 0;
	while(!d) {
		c=0;
		while(1) {
			if (costMatrix[r][c] == 0 && rowCover[r] == 0 && colCover[c] == 0){
				*rowStart = r;
				*colStart = c;
				d = 1;
			}
			c++;
			if(c >= colN || d) break;

		}
		r++;
		if(r >= rowN) d=1;
	}
}

int star_in_row(int row, int * c, float costMatrix[][NUM_BLOBS_MAX]) {
	int i;
	for(i = 0; i < curStar; i++) {
		if (stars[i][0] == i) {
			*c = stars[i][1];
			return 1;
		}
	}
	return 0;
}

static short path_row_0;
static short path_col_0;

void four(float costMatrix[][NUM_BLOBS_MAX], int rows, int cols) {
	short r = -1;
	short c = -1;
	short d = 0;
	while(!d) {
		find_a_zero(&r, &c, costMatrix, rows, cols);
		if(r == -1) {
			d = 1; step = 6;
			return;
		}
		primes[curPrime][0] = r;
		primes[curPrime][1] = c;
		curPrime++;
		int starCol;
		if(star_in_row(r, &starCol, costMatrix)) {
			rowCover[r] = 1;
			colCover[starCol] = 0;
			continue;
		}
		d = 1;
		step = 5;
		path_row_0 = r;
		path_col_0 = c;
	}
}

void five(float costMatrix[][NUM_BLOBS_MAX], int rows, int cols) {
	short d = 0;
	short r = -1;
	short c = -1;
	short path_count = 1;

}
*/
