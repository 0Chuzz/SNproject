/*
 * reflabeler.c
 *
 *  Created on: Jun 3, 2014
 *      Author: chuzz
 */
#include <assert.h>
#include "ppmencoder.h"
#include "imageiter.h"

unsigned char * reflabel(unsigned char *dst, unsigned char *src)
{
	int i,j;
	int used = 1;
	unsigned char labels[256];
	unsigned char *inter;
	unsigned char chosen;
	for(i=1; i<256; i++) labels[i] = i;

	expandBW(dst, src, 320, 240);

	inter = dst;
	for(j = 0; j < 240; j++){
		for(i = 0; i < 320; i++){
			if (inter[j * 320 + i] != 0){
				unsigned char leftpix =  i > 0 ? inter[j * 320 + i - 1] : 0 ;
				unsigned char uppix = j > 0 ? inter[(j-1)*320 + i] : 0;

				if (leftpix){
					if (uppix){
						int i;
						unsigned char oldlabel = labels[leftpix];
						for(i = 1; i < used; i++){
							if (labels[i] == oldlabel){
								labels[i] = labels[uppix];
							}
						}
					}
					chosen = labels[leftpix];
				} else if (uppix){
					chosen = labels[uppix];
				} else {
					chosen = labels[used++];
					assert(used < 256);
				}
				inter[j*320 + i] = chosen;
			}
		}
	}

	for(j = 0; j < 240; j++){
		for(i = 0; i < 320; i++){
			inter[j * 320 + i] = labels[inter[j*320 + i]];
		}
	}
	return inter;

}
