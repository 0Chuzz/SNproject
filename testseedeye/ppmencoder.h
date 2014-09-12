/*
 * ppmencoder.h
 *
 *  Created on: Jun 1, 2014
 *      Author: chuzz
 */

#ifndef PPMENCODER_H_
#define PPMENCODER_H_
#include "utils.h"

// scrive immagine etichettata in file ppm
void saveLabeled(const char filename[], label_t buff[], int w, int h);

// trasforma bitimg_t a label_t senza separare niente
int expandBW(label_t dst[], bitimg_t buff[], int w, int h);

#endif /* PPMENCODER_H_ */
