/*
 * imageiter.h
 *
 *  Created on: May 31, 2014
 *      Author: chuzz
 */

#ifndef IMAGEITER_H_
#define IMAGEITER_H_


typedef struct iter_s {
	unsigned int maxi, maxj;
	unsigned int i, j;
	unsigned char mask;
} iter_t;

void newMask(iter_t *it, int maxi, int maxj);
char up(iter_t *it);
char down(iter_t *it);
char left(iter_t *it);
char right(iter_t *it);
char nextLtr(iter_t *it);
char nextTopdown(iter_t *it);
char deref(unsigned char buff[], iter_t *it);


#endif /* IMAGEITER_H_ */
