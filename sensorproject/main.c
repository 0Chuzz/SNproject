/*
 * main.c
 *
 *  Created on: May 22, 2014
 *      Author: chuzz
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pbmdecoder.h"
#include "pngdecoder.h"
#include "ppmencoder.h"
#include "reflabeler.h"

static unsigned char bitbuffer[320 * 240 / 8];
static unsigned char expanded[320 * 240];

int main(int argn, char *argv[]) {
	FILE *f;
	unsigned char *asd;
	//char filename[2048];
	unsigned char *extension = argv[1] + strlen(argv[1]) - 4;
	if (!strcmp(extension, ".pbm")) {
		f = fopen(argv[1], "r");
		decodepbm(bitbuffer, 320, 240, f);
		fclose(f);

	} else if (!strcmp(extension, ".png")) {
		decodePng(argv[1], bitbuffer, 320, 240);
	} else {
		printf("error file format");
		exit(1);
	}
	asd = bitbuffer;

	//strcpy(filename, argv[1]);
	//strcat(filename, "-labeled.ppm");
	f = fopen("testunlabeled.pbm", "w");
	fwrite("P4\n320 240\n", 11, 1, f);
	fwrite(asd, 320 * 240 / 8, 1, f);
	fclose(f);
	//expandBW(expanded, asd, 320, 240);
	reflabel(expanded, asd);
	saveLabeled("testlabeled.ppm", expanded, 320, 240);
	return 0;
}
