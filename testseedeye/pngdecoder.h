/*
 * pngdecoder.h
 *
 *  Created on: Jun 5, 2014
 *      Author: chuzz
 */

#ifndef PNGDECODER_H_
#define PNGDECODER_H_
#include "utils.h"

// legge file in formato png
int decodePng(const char filename[], bitimg_t buff[], int w, int h);

#endif /* PNGDECODER_H_ */
