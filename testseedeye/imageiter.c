/*
 * imageiter.c
 *
 *  Created on: May 31, 2014
 *      Author: chuzz
 *
 *  IGNORA iteratore per bit ma non lo uso piu
 */

#include "imageiter.h"


void newMask(iter_t *ret, int maxi, int maxj){
	ret->maxi = maxi;
	ret->maxj = maxj;
	ret->i = 0;
	ret->j = 0;
	ret->mask= 0x80;
}

char up(iter_t *it){
	if(it->j-- == 0) return 0;
	else return 1;
}

char down(iter_t *it){
	if (it->j++ == it->maxj) return 0;
	else return 1;
}

char left(iter_t *it)  {
	if (it->mask != 0x80) it->mask <<= 1;
	else {
		if (it->i <= 0) return 0;
		it->i -= 8;
		it->mask = 0x01;
	}
	return 1;
}

char right(iter_t *it){
	if (it->mask != 0x01) it->mask >>= 1;
	else {
		if (it->i >= it->maxi) return 0;
		it->i += 8;
		it->mask = 0x80;
	}
	return 1;
}

char nextLtr(iter_t *it){
	if (it->mask != 0x01) it->mask >>= 1;
	else {
		it->mask = 0x80;
	}
	it->i++;
	if (it->i == it->maxi){
		it->i = 0;
		it->mask = 0x80;
		it->j++;
		if(it->j == it->maxj) return 0;
	}

	return 1;
}

char nextTopdown(iter_t *it){
	if (it->j++ == it->maxj){
		it->j = 0;
		return right(it);
	}
	return 1;
}


char deref(unsigned char buff[], iter_t *it){
	return (buff[it->j * (it->maxi/8) + (it->i/8)] & it->mask);
}


