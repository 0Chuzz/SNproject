/*
 * main.c
 *
 *  Created on: May 22, 2014
 *      Author: chuzz
 */
#define FINAL
#define TIME_CHECK
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mainLaptop.h"
#ifndef FINAL
#include <dirent.h>
#include "pbmdecoder.h"
#include "pngdecoder.h"
#include "ppmencoder.h"
#else
#include "util.h"
#endif
#include "utils.h"
#include "reflabeler.h"
#include "sievelabeler.h"
#include "labirinthlabeler.h"
#include "kalman2.h"
#include "minassign.h"
#include "mcu/microchip_pic32/inc/ee_timer.h"
#define VISIBLE_TRESHOLD 2



// buffer con immagine B/N, 1 bit per pixel
static bitimg_t bitbuffer[320 * 240 / 8];

static kalmanTrack states[NUM_BLOBS_MAX];

static point centroids[NUM_BLOBS_MAX];


#ifndef FINAL
int isPermutation(size_t permSize, int permutation[]) {
	int mapping[NUM_BLOBS_MAX];
	int i;
	for(i = 0; i < NUM_BLOBS_MAX;i++) {
		mapping[i] = 0;
	}
	for(i = 0; i < permSize; i++) {
		if(permutation[i] <= 0) continue;
		if (mapping[permutation[i]] == 0)
			mapping[permutation[i]] = 1;
		else {
			printf("Conflict on %d", permutation[i]);
			return 0;
		}
	}
	return 1;
}
#endif
//Test 2d kalman filter
int test2dKalman(){
	int i,j;
		point p;
		p.X = 0;
		p.Y = 0;
		kalmanTrack t;
		for(i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++)
				t.cov[i][j] = 0;
		}
		t.posX = 0;
		t.posY = 0;

		t.velX = 0;
		t.velY = 0;
		for(i = 0; i < 10; i++) {
			int r1 = rand() % 2;
			p.X+=(r1) ? 2 : 1;
			r1 = rand()%2;
			p.Y+=(r1) ? 4 : 2;
			predict(&t);
			printf("Prediction says:\n");
			printf("Pos: (%d, %d)\n", t.posX, t.posY);
			printf("Speed: (%.2f, %.2f)\n", t.velX, t.velY);
			int k;
			for(j = 0; j < 4; j++) {
				for(k = 0; k < 4; k++) {
					printf("%.2f \t", t.cov[j][k]);
				}
				printf("\n");
			}
			update(&t, p);
			printf("RealPosition is %d, %d\n", p.X, p.Y);
		}


		return 0;
}

void copyPermute(int permSize, label_t toPerm[WIDTH*HEIGHT], unsigned char*finalImg[WIDTH*HEIGHT+1], int permutation[NUM_BLOBS_MAX]) {
	int i;
	for(i = 0; i < WIDTH*HEIGHT; i++) {
		if (toPerm[i] > 0 && permutation[toPerm[i]] > 0) {
				*finalImg[i] = states[permutation[toPerm[i]]].id;
		} else {
			*finalImg[i] = 0;
		}
	}
	finalImg[WIDTH*HEIGHT] = '\0';
}
#ifndef FINAL
label_t * permute(size_t width, size_t height, size_t permsize, label_t toPerm[width*height], int permutation[permsize]) {
	
	int i;
	
	for(i = 0; i < WIDTH*HEIGHT; i++) {
	
		if (toPerm[i] > 0 && permutation[toPerm[i]] > 0) {
			toPerm[i] = states[permutation[toPerm[i]]].id + 1;
		} else {
			toPerm[i] = 0;
		}
	}
	
	return toPerm;
}
#endif

int mainLaptop(int argn, char *argv[]) {

	int stream_element = 0;
	int readed;
	/** First reading: used for the first estimation*/
	int i;
	#ifndef FINAL
	FILE *f;
	initializeDirectory(argv[1], argv[2], atoi(argv[3]), argv[4]);
	char outfile[1100];

	if(!nextImageFile(outfile)){
		printf("Empty Stream \n");
		exit(-1);
	}
	if(isPBM(outfile)) {
		f = fopen(outfile, "r");
		decodepbm(bitbuffer, 320, 240, f);
		fclose(f);
	} else decodePng(outfile, bitbuffer, 320, 240);
	#else
	readed = myread(bitbuffer, 320*240/8);
	myprintf("%16d", readed);
	myprintf("I readed %d bytes in total. Lol.\n",readed);
	#endif
	int numTracks = efficientKalmanCentroids(WIDTH, HEIGHT,bitbuffer, centroids);
	for (i = 0; i < numTracks; i++) {
		states[i].posX = centroids[i].X;
		states[i].posY = centroids[i].Y;
		states[i].velX = 0;
		states[i].velY = 0;
		states[i].id = assign(); //assign an identifier.
		states[i].age = 1;
		states[i].totalVisibleCount = 0;
		states[i].consecutiveInvisibleCount = 1;
		int j, k;
		for(j = 0; j < 4; j++)
			for(k = 0; k < 4; k++) {
			if(j == k) states[i].cov[j][k] = 1000;
			else states[i].cov[j][k] = 0;
		}
	}
	#ifdef FINAL
	for (i = 0; i < numTracks; i++) {
		set_at(bitbuffer, states[i].posX, states[i].posY);
	}
	myprintf("$$");
    //mywrite(bitbuffer, 320*240/8);
	while(1) {
		readed = myread(bitbuffer, 320*240/8);
		myprintf("%16d", readed);
	#else
	while(nextImageFile(outfile)) { //Inside we have the stream..!
		/** Read new file*/
		printf("Reading %s", outfile);
		if(isPBM(outfile)) { //remember to optimize
			f = fopen(outfile, "r");
			decodepbm(bitbuffer, 320, 240, f);
			fclose(f);
		} else decodePng(outfile, bitbuffer, 320, 240);
	#endif
	/* Apply labeling*/
		//myprintf("PREDICTION STARTS");
	#ifdef TIME_CHECK
		EE_UINT32 time_tot, time1;
		time_tot = time1 = get_time_stamp();
	#endif
		numTracks = predictAll(numTracks, states);
	#ifdef TIME_CHECK
		myprintf("Prediction\t%d\n", get_time_stamp() - time1);
	#endif
		//myprintf("PREDICTION ENDS");
	#ifndef FINAL
		for(i = 0; i < numTracks; i++) {
			printf("%d has foreseen position (%d,%d) and speed (%.2f, %.2f)\n", states[i].id, states[i].posX, states[i].posY, states[i].velX, states[i].velY);
		}
	#endif
		/** Calculate new centroids */
		//myprintf("Centroid calculation starts\n");
	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
		int numBlobs = efficientKalmanCentroids(WIDTH, HEIGHT, bitbuffer, centroids);
	#ifdef TIME_CHECK
		myprintf("Labelling+Centroids\t%d\n", get_time_stamp() - time1);
	#endif
		//myprintf("Centroids calculation ended\n");
		//for(i = 0; i < numBlobs; i++) {
		//	myprintf("%d blob has (%d, %d) and bounding box (%d,%d), (%d,%d)\n", i, centroids[i].X, centroids[i].Y, centroids[i].topX, centroids[i].topY, centroids[i].botX, centroids[i].botY);
		//}
	int permutation[NUM_BLOBS_MAX];
	for(i = 0; i < numBlobs; i++) permutation[i] = -1;
	short unassignedCols[NUM_BLOBS_MAX], unassignedColNum;
	//myprintf("Assignment starts\n");
	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
	findAssignment(numTracks, numBlobs, states, centroids, 100, permutation, unassignedCols, &unassignedColNum);
	#ifdef TIME_CHECK
		myprintf("Assignment\t%d\n", get_time_stamp() - time1);
	#endif
	//myprintf("Assignment ended\n");
	/** Assignment has been found: show */
	#ifndef FINAL
		if(!isPermutation(numBlobs, permutation)) exit(0);

		int colorPermutation[numBlobs];
		for(i = 0; i < numBlobs; i++) {
			colorPermutation[i] = states[permutation[i]].id;
		}
	#endif
	#ifndef FINAL
		if(!isPermutation(numBlobs, permutation)) exit(0);
	#endif
	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
		correctAll(numTracks, numBlobs, states, centroids, permutation);
	#ifdef TIME_CHECK
		myprintf("Correction\t%d\n", get_time_stamp() - time1);
	#endif
		//myprintf("Correction End\n");
		/** New assignments for the remaining */
		//printf("Need to assign new %d blobs:\n", unassignedColNum);
		for(i = 0; i < unassignedColNum; i++) {
			states[numTracks+i].id = assign(); //assign is broken, fix it.
			states[numTracks+i].posX = centroids[unassignedCols[i]].X;
			states[numTracks+i].posY = centroids[unassignedCols[i]].Y;
			states[numTracks+i].velX = states[numTracks+i].velY = 0;
			states[numTracks+i].consecutiveInvisibleCount = 1;
			states[numTracks+i].totalVisibleCount=0;
			states[numTracks+i].age = 1;
			int j, k;
			for(j = 0; j < 4; j++)
				for(k = 0; k < 4; k++) {
					if(j == k) states[numTracks+i].cov[i][j] = 1000;
					else states[numTracks+i].cov[i][j] = 0;
				}
		}
		//myprintf("New track assignment ended!");
		numTracks += unassignedColNum;
	#ifndef FINAL
		sprintf(outfile, "%s/%d.ppm", argv[5], stream_element);
		printf("%s\n", outfile);
		saveLabeled(outfile, permute(WIDTH, HEIGHT, numBlobs, expanded, permutation), WIDTH, HEIGHT);
	#else
		//unsigned char finalImg[320*240+1];
		//copyPermute( numBlobs, expanded, finalImg, permutation);
		//myprintf("Getting ready to send image in output..\n");
		/*
		for (i = 0; i < numTracks; i++) {
			set_at(bitbuffer, states[i].posX, states[i].posY);
		}*/
		//myprintf("Image is ready to be send \n");


		//Instead of image, send quintuple in the form:
		// <idT, x1,y1, x2, y2> for the bounding box of each blob recognized
		myprintf("Total\t%d\n", get_time_stamp() - time_tot);
		myprintf("$");

		for(i = 0; i < numBlobs; i++) {
			if(permutation[i] >= 0) {
				myprintf(" ");
				myprintf("%3d", states[permutation[i]].id);
				myprintf("%3d", centroids[i].topX);
				myprintf("%3d", centroids[i].topY);
				myprintf("%3d", centroids[i].botX);
				myprintf("%3d", centroids[i].botY);
			}
		}
		myprintf("$");
	    //mywrite(bitbuffer, 320*240/8);

	#endif
		stream_element++;

	}
	return 0;
}
	/*

	//the noise in the system
	float Q = 2;
	float R = 1;

	initializeDirectory(argv[1], argv[2], atoi(argv[3]), argv[4]);
	int stream_element = 0;
	FILE *f;
	char outfile[1100];
	/** First reading: used for the first estimation
	int i;
	if(!nextImageFile(outfile)){
		printf("Empty Stream \n");
		exit(-1);
	}

	if(isPBM(outfile)) {
		f = fopen(outfile, "r");
		decodepbm(bitbuffer, 320, 240, f);
		fclose(f);
	}else decodePng(outfile, bitbuffer, 320, 240);

	labirLabel(expanded, bitbuffer);

	int numTracks = kalmanCentroids(expanded, WIDTH, HEIGHT, centroids);

	for (i = 0; i < numTracks; i++) {
		states[i].posX = centroids[i].X;
		states[i].posY = centroids[i].Y;
		states[i].velX = 0;
		states[i].velY = 0;
		states[i].id = assign(); //assign an identifier.
		states[i].age = 1;
		states[i].totalVisibleCount = 1;
	}
	//saveLabeled("/tmp/start.ppm", addCentroids(expanded, states, centroids, numTracks), WIDTH, HEIGHT);
	//Now build the system after the centroids!

	while(nextImageFile(outfile)) { //Inside we have the stream..!

		/** Read new file
		if(isPBM(outfile)) { //remember to optimize
			f = fopen(outfile, "r");
			decodepbm(bitbuffer, 320, 240, f);
			fclose(f);
		} else decodePng(outfile, bitbuffer, 320, 240);

		/** Apply labeling
		labirLabel(expanded, bitbuffer);

		predict(states, Q, R, &numTracks); //Predict also eliminates tracks lost.
		for(i = 0; i < numTracks; i++) {
			printf("Track %d prediction is (%d,%d)\n", states[i].id, states[i].posX, states[i].posY);
		}

		int numBlobs = kalmanCentroids(expanded, WIDTH, HEIGHT, centroids);
		for(i = 0; i < numBlobs; i++) {
				printf("Blob %d: (%d, %d)\n", i, centroids[i].X, centroids[i].Y);
		}

		float costMatrix[numTracks][numBlobs];
		int permutation[NUM_BLOBS_MAX];
		for(i = 0; i < numBlobs; i++) permutation[i] = 0;
		//Compute the cost matrix
		short unassignedRows[numTracks], unassignedCols[numBlobs], unassignedRowNum, unassignedColNum;
		findAssignment(numTracks, numBlobs, states, centroids, 70, permutation, unassignedCols, &unassignedColNum);
	/*
		computeCostMatrix(states, numTracks, centroids, numBlobs, costMatrix);
		minAssignHeuristic(numTracks, numBlobs, costMatrix, permutation, unassignedRows, &unassignedRowNum, unassignedCols, &unassignedColNum);

		/** Deal with unassigned tracks ?
		 * 1. Increment invisible count --> already done in predict
		 * 2. Increment age (it is done for all) --> already done in predict
		 * ---> Seems like there's nothing to be done..!
		 *  */

		/** Deal with unassigned blobs
		 * If a blob was not assigned, we need to create a new track for it.
		 * */

		/** Recalculate permutation, being aware that if the age is less than the
		 * treshold, we shall not do it!

		if(!isPermutation(numBlobs, permutation)) exit(0);

		//Update current number of tracks..!
		correct(states, permutation, centroids, numTracks, numBlobs);
		printf("Need to assign %d cols!\n", unassignedColNum);
		for(i = 0; i < unassignedColNum; i++) {
			states[numTracks+i].id = assign(); //assign is broken, fix it.
			states[numTracks+i].posX = centroids[unassignedCols[i]].X;
			states[numTracks+i].posY = centroids[unassignedCols[i]].Y;
			states[numTracks+i].velX = states[numTracks+i].velY = 0;
			states[numTracks+i].consecutiveInvisibleCount = 1;
			states[numTracks+i].age++;
		}
		printf("assigned\n");
		numTracks += unassignedColNum;
		sprintf(outfile, "%s/%d.ppm", argv[5], stream_element);
		printf("assigned\n");
		saveLabeled(outfile, permute(expanded, permutation, WIDTH, HEIGHT), WIDTH, HEIGHT);

		for(i = 0; i < 320*240;i++) {
			expanded[i] = 0;
		}
		stream_element++;
		//if(stream_element > 15) return 0;
	}

	printf("Finished!\n");
	return 0;


	return 0;*/