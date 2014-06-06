/*
 * pbmdecoder.c
 *
 *  Created on: May 22, 2014
 *      Author: chuzz
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "pbmdecoder.h"

//static unsigned char buffer[240 * 320 / 8];



int decodepbm(unsigned char *buffer, int bw, int bh, FILE* f) {
	int w, h;
	int i, j;
	char col;

	fscanf(f, "P4\n%d %d\n", &w, &h);
	if (w < bw || h < bh)
		return -1;

	for (j = 0; j < bh; j++) {
		for (i = 0; i < BYTES_FOR_BITS(bw); i++) {
			fread(&col, 1, 1, f);
			buffer[j * BYTES_FOR_BITS(bw) + i] = col;
		}
		while (i < BYTES_FOR_BITS(w)) {
			fread(&col, 1, 1, f);
			i++;
		}
	}

	return 0;
}
