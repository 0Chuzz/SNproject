/*
 * utils.c
 *
 *  Created on: Jun 5, 2014
 *      Author: chuzz
 */
#include <stdio.h>
#include "utils.h"


void dump_bitimg(char filename[], bitimg_t buff[]){
	FILE *f;

	f = fopen(filename, "w");
	fwrite("P4\n320 240\n", 11, 1, f);
	fwrite(buff, 320 * 240 / 8, 1, f);
	fclose(f);
}
