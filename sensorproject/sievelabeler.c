/*
 * sievelabeler.c
 *
 *  Created on: Jun 6, 2014
 *      Author: chuzz
 */

#include "utils.h"

#define LEFT(buff) ((i > 0)? at(buff, i-1, j) : 0)
#define UP(buff) ((j> 0)? at(buff, i, j-1) : 0)
#define ENDPASS 0
#define MOREPASS 1

int sieve_pass(unsigned char *from, unsigned char *in) {
	int i, j;
	unsigned char move = 0;
	int ret = ENDPASS;

	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {
			if (at(from, i, j)) {
				if (UP(in)) {
					if (LEFT(from)) {
						ret = MOREPASS;
					}
					move = 1;
				} else if (LEFT(in)) {
					if (UP(from)) {
						ret = MOREPASS;
					}
					move = 1;
				} else {
					move = 0;
				}
				if (move) {
					clear_at(from, i, j);
					set_at(in, i, j);
				}
			}
		}
	}
	return ret;
}

#define RIGHT(buff) ((i < WIDTH - 1)? at(buff, i+1, j) : 0)
#define DOWN(buff) ((j < HEIGHT - 1)? at(buff, i, j+1) : 0)

int sieve_counterpass(unsigned char *from, unsigned char *in) {
	int i, j;
	unsigned char move = 0;
	int ret = ENDPASS;

	for (j = HEIGHT -1; j >= 0; j--) {
		for (i = WIDTH-1; i >= 0; i--) {
			if (at(from, i, j)) {
				if (DOWN(in)) {
					if (RIGHT(from)) {
						ret = MOREPASS;
					}
					move = 1;
				} else if (RIGHT(in)) {
					if (DOWN(from)) {
						ret = MOREPASS;
					}
					move = 1;
				} else {
					move = 0;
				}
				if (move) {
					clear_at(from, i, j);
					set_at(in, i, j);
				}
			}
		}
	}
	return ret;
}


int sieve_extract(unsigned char *from, unsigned char *to) {
	int i, j;
	int asd;
	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {
			if (at(from, i, j)) {
				clear_at(from, i, j);
				set_at(to, i, j);
				j = HEIGHT;
				i = WIDTH;
			}
		}
	}
	if (j == HEIGHT && WIDTH == i)
		return 1;

	do {
		asd = sieve_pass(from, to);
		if (asd == MOREPASS) {
			asd = sieve_counterpass(from, to);
		}
	} while (asd == MOREPASS);
	return 0;
}

void sieveLabel(unsigned char *expanded, unsigned char *bw) {
	unsigned char to[BYTES_FOR(WIDTH) * HEIGHT];
	int i, j;
	unsigned char label = 1;

	for(i = 0; i < sizeof to; i++) to[i] = 0;
	while (!sieve_extract(bw, to)) {
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
