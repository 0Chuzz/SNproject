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

static int tempi, tempj;
int sieve_pass(unsigned char *from, unsigned char *in, int starti, int startj);
int sieve_counterpass(unsigned char *from, unsigned char *in, int starti, int startj);

int sieve_pass(unsigned char *from, unsigned char *in, int starti, int startj) {
	int i, j;
	unsigned char endflag = 0;
	int ret = ENDPASS;

	for (j = startj; j < HEIGHT; j++) {

		for (i = starti; i < WIDTH; i++) {
			if (at(from, i, j)) {
				if (UP(in)) {
					clear_at(from, i, j);
					set_at(in, i, j);
					if (LEFT(from)) {
						sieve_counterpass(from, in, i-1, j);
					}
					endflag = 0;
				} else if (LEFT(in)) {
					clear_at(from, i, j);
					set_at(in, i, j);
					if (UP(from)) {
						sieve_counterpass(from, in, i, j-1);
					}
					endflag = 0;
				}
			}
		}
		if (endflag) break;
		else endflag = 1;
	}
	return ret;
}

#define RIGHT(buff) ((i < WIDTH - 1)? at(buff, i+1, j) : 0)
#define DOWN(buff) ((j < HEIGHT - 1)? at(buff, i, j+1) : 0)

int sieve_counterpass(unsigned char *from, unsigned char *in, int starti, int startj) {
	int i, j;
	unsigned char endflag = 0;
	int ret = ENDPASS;

	for (j = startj; j >= 0; j--) {
		for (i = starti; i >= 0; i--) {
			if (at(from, i, j)) {
				if (DOWN(in)) {
					clear_at(from, i, j);
					set_at(in, i, j);
					if (RIGHT(from)) {
						sieve_pass(from, in, i+1, j);
					}
					endflag = 0;
				} else if (RIGHT(in)) {
					clear_at(from, i, j);
					set_at(in, i, j);
					if (DOWN(from)) {
						sieve_pass(from, in, i, j+1);
					}
					endflag = 0;
				}
			}
		}
		if (endflag) break;
		else endflag = 1;
	}

	return ret;
}


int sieve_extract(unsigned char *from, unsigned char *to) {
	int i, j;
	int asd;
	char foundflag = 0;
	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {
			if (at(from, i, j)) {
				clear_at(from, i, j);
				set_at(to, i, j);
				tempj = j;
				tempi = i;
				j = HEIGHT;
				i = WIDTH;
				foundflag = 1;
			}
		}
	}
	if (!foundflag)
		return 1;

	do { // XXX useless with recursive pass, but needs to return iterative so it stays
		asd = sieve_pass(from, to, tempi, tempj);
		if (asd == MOREPASS) {
			asd = sieve_counterpass(from, to, tempi, tempj);
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
