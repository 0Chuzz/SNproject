/*
 * pbmdecoder.h
 *
 *  Created on: May 22, 2014
 *      Author: chuzz
 */

#ifndef PBMDECODER_H_
#define PBMDECODER_H_

#include<stdlib.h>
#include "utils.h"

// legge i file in formato pbm, salva in buff
int decodepbm(bitimg_t buff[], int w, int h, FILE* f);


#endif /* PBMDECODER_H_ */
