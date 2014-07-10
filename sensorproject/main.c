/*
 * main.c
 *
 *  Created on: May 22, 2014
 *      Author: chuzz
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "pbmdecoder.h"
#include "pngdecoder.h"
#include "ppmencoder.h"
#include "reflabeler.h"
#include "sievelabeler.h"


// buffer con immagine B/N, 1 bit per pixel
static bitimg_t bitbuffer[320 * 240 / 8];

// buffer etichette finali 8 bit per pixel
static label_t expanded[320 * 240];


// argv[1] file da leggere
int main(int argn, char *argv[]) {
	FILE *f;
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


	//strcpy(filename, argv[1]);
	//strcat(filename, "-labeled.ppm");

	dump_bitimg("testunlabeled.pbm", bitbuffer);


	//expandBW(expanded, asd, 320, 240);
	//reflabel(expanded, asd);
	sieveLabel(expanded, bitbuffer);

	saveLabeled("testlabeled.ppm", expanded, WIDTH, HEIGHT);

	return 0;
}
