/*
 * labirintlabeler.c

 *
 *  Created on: Jul 7, 2014
 *      Author: chuzz
 *
 *
 *      IGNORA ci sto ancora lavorando
 */

#include "labirinthlabeler.h"
#include "utils.h"

typedef enum {
	UP,
	RIGHT,
	DOWN,
	LEFT,
	NUM_DIRS
} dir_t;

#define MOVE_DIR(dir) do{\
	switch(dir){\
	case UP : j--;break;\
	case RIGHT: i--;break;\
	case DOWN: j++; break;\
	case LEFT: i++; break;\
	case NUM_DIRS: break;\
	}} while (0)


int at_dir(bitimg_t *buff, int i, int j, dir_t dir){
	MOVE_DIR(dir);
	if (i < 0 || i >= WIDTH || j < 0 || j >= HEIGHT) return 0;
	return at(buff, i, j);
}

dir_t rightmost_unvisited(bitimg_t *from, bitimg_t *to, int i, int j, dir_t dir){
	dir_t ret = (dir +1) % NUM_DIRS;
	int x;
	for (x= 0; x < 3;ret = (ret - 1) % NUM_DIRS, x++){
		if (at_dir(from, i, j, ret) && !at_dir(to, i, j, ret)){
			return ret;
		}
	}
	return NUM_DIRS;
}

dir_t leftmost_visited(bitimg_t *from, bitimg_t *to, int i, int j, dir_t dir){
	dir_t ret = (dir - 2) % NUM_DIRS;
	int x;
	for (x= 0; x < 4;ret = (ret + 1) % NUM_DIRS, x++){
		if (at_dir(from, i, j, ret) && at_dir(to, i, j, ret)){
			return ret;
		}
	}
	return NUM_DIRS;
}

int labir_extract(bitimg_t *from, bitimg_t *to){
	int i, j;
	dir_t dir = RIGHT, newdir;
	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {
			if (at(from, i, j)) {
				goto found;
			}
		}
	}
	return 1;
	found:
	set_at(to, i, j);

	check_unvisited:
	newdir = rightmost_unvisited(from, to, i, j, dir);
	if(newdir != NUM_DIRS){
		dir = newdir;
		MOVE_DIR(dir);
		goto found;
	}

	clear_at(from, i, j);
	newdir = leftmost_visited(from, to, i, j, dir);
	if (newdir != NUM_DIRS){
			dir = newdir;
			MOVE_DIR(dir);
			goto check_unvisited;
	}
	return 0;

}

void labirLabel(label_t *expanded, bitimg_t *from)
{
	bitimg_t to [BYTES_FOR(WIDTH) * HEIGHT];
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
