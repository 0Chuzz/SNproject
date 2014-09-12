/*
 * sievelabeler.h
 *
 *  Created on: Jun 6, 2014
 *      Author: chuzz
 */
#ifndef SIEVELABELER_H_
#define SIEVELABELER_H_
#include "utils.h"

// estrai da immagine from una singola componente connessa, salva dentro to
int sieve_extract(bitimg_t from[], bitimg_t to[]);

// usa sieve_extract per etichettare tutte le componenti
void sieveLabel(label_t expanded[], bitimg_t bw[]);

#endif
