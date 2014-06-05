/*
 * ppmencoder.h
 *
 *  Created on: Jun 1, 2014
 *      Author: chuzz
 */

#ifndef PPMENCODER_H_
#define PPMENCODER_H_

void saveLabeled(const char filename[], unsigned char buff[], int w, int h);

int expandBW(unsigned char dst[], unsigned char buff[], int w, int h);

#endif /* PPMENCODER_H_ */
