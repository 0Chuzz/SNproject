/*
 * ppmencoder.c
 *
 *  Created on: Jun 1, 2014
 *      Author: chuzz
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ppmencoder.h"
#include "imageiter.h"

static const unsigned char colormap[][3] = {
		{0, 0, 0},
		{255, 0, 0},
		{0, 255, 0},
		{0, 0, 255},
		{100, 100, 100},
		{255, 100, 100},
		{100, 255, 100},
		{100, 100, 255},

};

//static unsigned char expanded[320*240];

void saveLabeled(const char filename[], unsigned char buff[], int w, int h){
	FILE *to = fopen(filename,"w");
	int i, j;

	fprintf(to, "P6\n%d %d 255\n" , w, h);
	for (j=0;j < h; j++)
		for (i = 0; i < w; i++){
			fwrite(colormap[buff[j*w + i]], 1, 3, to);
		}
	fclose(to);
}

int expandBW(unsigned char expanded[], unsigned char buff[], int w, int h){
	int i;
	iter_t it;
	newMask(&it, w, h);


	i=0;
	do{
		expanded[i++] = at(buff, &it) ? 1: 0;
	}while(nextLtr(&it));

	if (i != w * h ) return -1;

	return 0;
}
