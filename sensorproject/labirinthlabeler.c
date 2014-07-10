/*
 * labirintlabeler.c

 *
 *  Created on: Jul 7, 2014
 *      Author: chuzz
 */

#include "labirinthlabeler.h"
#include "utils.h"

typedef enum {
	UP,
	RIGHT,
	DOWN,
	LEFT
} dir_t;

int labir_extract(unsigned char *from, unsigned char *to){
	int i, j;
	dir_t dir = DOWN;
	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {
			if (at(from, i, j)) {
				goto found;
			}
		}
	}
	return 1;
	found:
	if(at(to, i, j)){
		clear_at(from, i, j);
	} else {
		set_at(to, i, j);
	}
	if(check_dir(to, dir) || check_dir(to,dir)){
		move_dir(dir);
		goto found;
	}

	return 0;

}

void labirLabel(unsigned char *expanded, unsigned char *from)
{
	unsigned char to [BYTES_FOR(WIDTH) * HEIGHT];
	int i,j;
	unsigned char label = 1;

	for(i = 0; i < sizeof to; i++) to[i] = 0;
	while (!labir_extract(from, to)) {
		for (j = 0; j < HEIGHT; j++) {
			for (i = 0; i < WIDTH; i++) {
				if (at(to, i, j)) {
					expanded[j*WIDTH + i] = label;
				}
			}
		}
		label++;
		for(i = 0; i < sizeof to; i++) to[i] = 0;
	}
}
