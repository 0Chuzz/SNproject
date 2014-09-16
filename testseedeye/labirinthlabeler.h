/*
 * labirinthlabeler.h
 *
 *  Created on: Jul 7, 2014
 *      Author: chuzz
 *
 *       IGNORA ci sto lavorando
 */

#ifndef LABIRINTHLABELER_H_
#define LABIRINTHLABELER_H_
#include "utils.h"

int labir_extract(bitimg_t *from, bitimg_t *to);
void labir_init2();
int labir_extract2(bitimg_t *from, int *centrX, int *centrY,
		int *topX, int *topY, int*botX, int *botY);
void labirLabel(label_t *expanded, bitimg_t *from);

#endif /* LABIRINTHLABELER_H_ */
