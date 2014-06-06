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

static unsigned char reverse(unsigned char b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

static void flip(unsigned char *buff) {
	int i;
	unsigned char tmp;
	const int len = (BYTES_FOR(WIDTH) * HEIGHT);
	for (i = 0; i < len / 2; i++) {
		tmp = buff[i];
		buff[i] = reverse(buff[len - i]);
		buff[len - i] = reverse(tmp);
	}
}

int sieve_extract(unsigned char *from, unsigned char *to) {
	int morepasses;
	int i, j;
	int flipped = 0;
	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {
			if (at(from, i, j)) {
				clear_at(from, i, j);
				set_at(to, i, j);
				break;
			}
		}
	}
	if (j == HEIGHT && WIDTH == i)
		return 1;

	while ((morepasses = sieve_pass(from, to)) == MOREPASS) {
		flip(from);
		flip(to);
		flipped = ~flipped;
	}
	if (flipped) {
		flip(from);
		flip(to);
	}
	return 0;
}

void sieveLabel(unsigned char *expanded, unsigned char *bw) {
	unsigned char to[BYTES_FOR(WIDTH) * HEIGHT];
	int i, j;
	unsigned char label = 1;
	while (!sieve_extract(bw, to)) {
		for (j = 0; j < HEIGHT; j++) {
			for (i = 0; i < WIDTH; i++) {
				if (at(to, i, j)) {
					expanded[i*j] = label;
				}
			}
		}
		label++;
	}
}
