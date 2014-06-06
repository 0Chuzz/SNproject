/*
 * utils.h
 *
 *  Created on: Jun 5, 2014
 *      Author: chuzz
 */

#ifndef UTILS_H_
#define UTILS_H_

#define BYTES_FOR(x) (x/8 + (x%8? 1:0))

#define WIDTH 320
#define HEIGHT 240

inline unsigned char at(unsigned char buff[], unsigned int x, unsigned int y){
	return (buff[y * BYTES_FOR(WIDTH) + (x / 8)] & (0x80 >> (x % 8)));
}



#endif /* UTILS_H_ */
