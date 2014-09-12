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
/*static const unsigned char colormap[][3] = {
		{0, 0, 0},
		{0, 255, 0},
		{0, 0, 255},
		{100, 100, 100},
		{255, 100, 100},
		{100, 255, 100},
		{100, 100, 255},
		{100,255,255},
		{120,120,120},
		{150,200,204},
		{255, 0, 0}
};
*
void dump_bitimg(char filename[], bitimg_t buff[]){
	FILE *f;

	f = fopen(filename, "w");
	fwrite("P4\n320 240\n", 11, 1, f);
	fwrite(buff, 320 * 240 / 8, 1, f);
	fclose(f);
}
//Procedure to find the centroids of the blobs found
//works!
/*
int* findcentroids(label_t expanded[], int w, int h) {
	unsigned int count[256];
	unsigned int x_coord[256];
	unsigned int y_coord[256];
	int i,j;
	for(i = 0; i < 256; i++) {
		count[i] = 0;
		x_coord[i] = 0;
		y_coord[i] = 0;
	}
	printf("Initialized\n");
	for(j = 0; j < h; j++)
	for (i = 0; i < w; i++) {
		count[expanded[j*w+i]]++;
		x_coord[expanded[j*w+i]] += i;
		y_coord[expanded[j*w+i]] += j;
	}
	static int centroids[256];
	for(i = 0; i < 256; i++) {
		if(x_coord[i]*y_coord[i] > 0 && count[i] > 0) {
			int xcentr = (x_coord[i]/count[i]);
			int ycentr = (y_coord[i]/count[i]);
			centroids[i] = xcentr;
		}
	}
	return centroids;
}
static char prefixString[1000];
static int start = 0;
short open = 0;
static char suffix[5];
int initializeDirectory(const char directory[], const char prefix[], int s, const char suff[]) {
	open = 1;
	int allocateSize = strlen(directory)+strlen(prefix)+6;
	sprintf(prefixString, "%s/%s", directory, prefix);
	sprintf(suffix, ".%s", suff);
	start = s;
	return 0;
}

int file_exist (char *filename)
{
 // struct stat buffer;
 // return (stat (filename, &buffer) == 0);
	return 1;
}

int nextImageFile(char fileName[]) {
	if(open == 0) return 0;
	sprintf(fileName, "%s%d%s", prefixString,start++,suffix);
	if(file_exist(fileName))
		return 1;
	return 0;
}
int isPBM(const char filename[]) {
	unsigned char* ext = filename+strlen(filename)-4;
	return !strcmp(ext, ".pbm");
}
int isPNG(const char filename[]) {
	unsigned char* ext = filename+strlen(filename)-4;
	return !strcmp(ext, ".png");
}*/

