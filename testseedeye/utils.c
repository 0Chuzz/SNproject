/*
 * utils.c
 *
 *  Created on: Jun 5, 2014
 *      Author: chuzz
 */
//#include <stdio.h>
#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
#include "utils.h"
#define NUM_BLOBS_MAX 64
/**
 * Checks if the permutation of size permsize is actually a permutation
 * or if some repetitions do occour.
 */
int isPermutation(int permSize, int permutation[]) {
	int mapping[NUM_BLOBS_MAX];
	int i;
	for(i = 0; i < NUM_BLOBS_MAX;i++) {
		mapping[i] = 0;
	}
	for(i = 0; i < permSize; i++) {
		if(permutation[i] <= 0) continue;
		if (mapping[permutation[i]] == 0)
			mapping[permutation[i]] = 1;
		else {
			printf("Conflict on %d", permutation[i]);
			return 0;
		}
	}
	return 1;
}
/*
void copyPermute(int permSize, label_t toPerm[WIDTH*HEIGHT], unsigned char*finalImg[WIDTH*HEIGHT+1], int permutation[NUM_BLOBS_MAX]) {
	int i;
	for(i = 0; i < WIDTH*HEIGHT; i++) {
		if (toPerm[i] > 0 && permutation[toPerm[i]] > 0) {
				*finalImg[i] = states[permutation[toPerm[i]]].id;
		} else {
			*finalImg[i] = 0;
		}
	}
	finalImg[WIDTH*HEIGHT] = '\0';
}

label_t * permute(size_t width, size_t height, size_t permsize, label_t toPerm[width*height], int permutation[permsize]) {

	int i;

	for(i = 0; i < WIDTH*HEIGHT; i++) {

		if (toPerm[i] > 0 && permutation[toPerm[i]] > 0) {
			toPerm[i] = states[permutation[toPerm[i]]].id + 1;
		} else {
			toPerm[i] = 0;
		}
	}

	return toPerm;
}
*/
