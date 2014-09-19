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
#include "labirinthlabeler.h"
#include "kalman2.h"
#include "approximatedKalman.h"
#include "minassign.h"
#include "mcu/microchip_pic32/inc/ee_timer.h"
#define VISIBLE_TRESHOLD 2
#define TIME_CHECK
#define UNASSIGNMENT_COST 100

/** Management of approximated and unapproximated versions */
#ifdef APPROXIMATED


	#define TRACK	approxKalmanTrack_t
	#define PREDICT_ALL(N, S) approximate_predictAll((N), (S))
	#define FIND_ASSIGNMENT(nT, nB, S, C, uC, P, unC, unCNum)	fastFindAssignment((nT), (nB), (S), (C), (uC), (P), (unC), (unCNum))
	#define CORRECT_ALL(nT, nB, S, C, P)	approximate_correctAll((nT), (nB), (S), (C), (P))


#else

	#define TRACK kalmanTrack
	#define UNIT_VALUE 1

	#define PREDICT_ALL(N, S)	predictAll((N), (S))
	#define FIND_ASSIGNMENT(nT, nB, S, C, uC, P, unC, unCNum)	findAssignment((nT), (nB), (S), (C), (uC), (P), (unC), (unCNum))
	#define CORRECT_ALL(nT, nB, S, C, P)	correctAll((nT), (nB), (S), (C), (P))

#endif




// buffer con immagine B/N, 1 bit per pixel
static bitimg_t bitbuffer[320 * 240 / 8];

static TRACK states[NUM_BLOBS_MAX];

static point centroids[NUM_BLOBS_MAX];
/**
 * Per l'assegnamento di nuovi oggetti
 */
static int newId = 0;


/**
 * Executes the tracking procedure
 */
#ifndef FINAL
//Version for pc
int mainLaptop(int argn, char *argv[]) {
	/** First reading: used for the first estimation*/
	int i;
	int stream_element = 0;
	/**  Read from file into bitbuffer*/
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
	/** Detect initial blobs. Then initialize all tracks to  the blobs detected now */
	int numTracks = efficientKalmanCentroids(WIDTH, HEIGHT,bitbuffer, centroids);
	for (i = 0; i < numTracks; i++) {
		states[i].posX = centroids[i].X;
		states[i].posY = centroids[i].Y;
		states[i].velX = 0;
		states[i].velY = 0;
		states[i].id = newId++; //assign an identifier.
		states[i].age = 1;
		states[i].totalVisibleCount = 0;
		states[i].consecutiveInvisibleCount = 1;
		int j, k;
		/** The autocov. of the states (on the principal diagonal) is set
		 * to a large enough value, represented by INITIAL_AUTOCOV(=1000 in the ex.)
		 */
		for(j = 0; j < 4; j++)
			for(k = 0; k < 4; k++) {
			if(j == k) states[i].cov[j][k] = INITIAL_AUTOCOV*UNIT_VALUE;
			else states[i].cov[j][k] = 0;
		}
	}
	while(nextImageFile(outfile)) { //Inside we have the stream..!
		/** Read new file*/
		printf("Reading %s", outfile);
		if(isPBM(outfile)) { //remember to optimize
			f = fopen(outfile, "r");
			decodepbm(bitbuffer, 320, 240, f);
			fclose(f);
		} else decodePng(outfile, bitbuffer, 320, 240);
	#ifdef TIME_CHECK
		EE_UINT32 time_tot, time1;
		time_tot = time1 = get_time_stamp();
	#endif
		/** Perform a prediction on the next state of the current tracks */
		numTracks = PREDICT_ALL(numTracks, states);
	#ifdef DEBUG
		//Print all predictions
		for(i = 0; i < numTracks; i++) {
			myprintf("%d has foreseen position (%d,%d) and speed (%.2f, %.2f)\n", i, states[i].posX, states[i].posY, (float) states[i].velX/UNIT_VALUE, (float)states[i].velY/UNIT_VALUE);
		}
	#endif
	#ifdef TIME_CHECK
		myprintf("Prediction\t%d\n", elapsed_us(time1, get_time_stamp()));
	#endif

	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
		/** Detect current blobs and their centroids & bounding box */
		int numBlobs = efficientKalmanCentroids(WIDTH, HEIGHT, bitbuffer, centroids);

		#ifdef TIME_CHECK
			myprintf("Labeling+Centroids\t%d\n", elapsed_us(time1, get_time_stamp()));
		#endif
		/** Initialize permutation = vector used for blob -> track assignment
		 */
		int assignment[NUM_BLOBS_MAX];
		for(i = 0; i < NUM_BLOBS_MAX; i++) assignment[i] = -1;
		short unassignedCols[NUM_BLOBS_MAX], unassignedColNum;

	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
	/** Assignment heuristic.
	 * After the distance between foreseen centroid and actual centroid is bigger than UNASSIGNMENT_COST
	 * no assignment is performed.*/
	FIND_ASSIGNMENT(numTracks, numBlobs, states, centroids, UNASSIGNMENT_COST, assignment, unassignedCols, &unassignedColNum);
	#ifdef TIME_CHECK
		myprintf("Tracks\t%d\n", numTracks);
		myprintf("BBB\t%d\n", numBlobs);
	#endif


	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
		/** Perform a kalman correction */
		CORRECT_ALL(numTracks, numBlobs, states, centroids, assignment);
	#ifdef TIME_CHECK
		myprintf("Correction\t%d\n", elapsed_us(time1, get_time_stamp()));
		time1 = get_time_stamp();
	#endif
		for(i = 0; i < unassignedColNum; i++) {
			states[numTracks+i].id = newId++; //assign is broken, fix it.
			states[numTracks+i].posX = centroids[unassignedCols[i]].X;
			states[numTracks+i].posY = centroids[unassignedCols[i]].Y;
			states[numTracks+i].velX = states[numTracks+i].velY = 0;
			states[numTracks+i].consecutiveInvisibleCount = 1;
			states[numTracks+i].totalVisibleCount=0;
			states[numTracks+i].age = 1;
			int j, k;
			for(j = 0; j < 4; j++)
				for(k = 0; k < 4; k++) {
					if(j == k) states[numTracks+i].cov[j][j] = INITIAL_AUTOCOV*UNIT_VALUE;
					else states[numTracks+i].cov[j][k] = 0;
				}
		}
	#ifdef TIME_CHECK
		myprintf("New tracks creation\t%d\n", elapsed_us(time1, get_time_stamp()));
	#endif

		numTracks += unassignedColNum;
		sprintf(outfile, "%s/%d.ppm", argv[5], stream_element);
		printf("%s\n", outfile);
		saveLabeled(outfile, permute(WIDTH, HEIGHT, numBlobs, expanded, assignment), WIDTH, HEIGHT);
		stream_element++;

		stream_element++;
	}
	return 0;
}
#else
int mainLaptop(int argn, char *argv[]) {

	/** First reading: used for the first estimation*/
	int i;
	int stream_element = 0;
	/** Receive from serial into bitbuffer */
	int readed;
	unsigned char buffer[10];
	myread(buffer,10);
	int maxStreamLen = atoi(buffer);
	readed = myread(bitbuffer, 320*240/8);
	myprintf("%16d", readed);
	/** Detect initial blobs. Then initialize all tracks to  the blobs detected now */
	int numTracks = efficientKalmanCentroids(WIDTH, HEIGHT,bitbuffer, centroids);
	for (i = 0; i < numTracks; i++) {
		states[i].posX = centroids[i].X;
		states[i].posY = centroids[i].Y;
		states[i].velX = 0;
		states[i].velY = 0;
		states[i].id = newId++; //assign an identifier.
		states[i].age = 1;
		states[i].totalVisibleCount = 0;
		states[i].consecutiveInvisibleCount = 1;
		int j, k;
		/** The autocov. of the states (on the principal diagonal) is set
		 * to a large enough value, represented by INITIAL_AUTOCOV(=1000 in the ex.)
		 */
		for(j = 0; j < 4; j++)
			for(k = 0; k < 4; k++) {
			if(j == k) states[i].cov[j][k] = INITIAL_AUTOCOV*UNIT_VALUE;
			else states[i].cov[j][k] = 0;
		}
	}
	//Notify the reader that we have no bounding box nor any other information
	//and require another image.
	myprintf("$$");
	while(--maxStreamLen) {
		readed = myread(bitbuffer, 320*240/8);
		myprintf("%16d", readed);
	#ifdef TIME_CHECK
		EE_UINT32 time_tot, time1;
		time_tot = time1 = get_time_stamp();
	#endif
		/** Perform a prediction on the next state of the current tracks */
		numTracks = PREDICT_ALL(numTracks, states);
	#ifdef DEBUG
		//Print all predictions
		for(i = 0; i < numTracks; i++) {
			myprintf("%d has foreseen position (%d,%d) and speed (%.2f, %.2f)\n", i, states[i].posX, states[i].posY, (float) states[i].velX/UNIT_VALUE, (float)states[i].velY/UNIT_VALUE);
		}
	#endif
	#ifdef TIME_CHECK
		myprintf("Prediction\t%d\n", elapsed_us(time1, get_time_stamp()));
	#endif

	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
		/** Detect current blobs and their centroids & bounding box */
		int numBlobs = efficientKalmanCentroids(WIDTH, HEIGHT, bitbuffer, centroids);

		#ifdef TIME_CHECK
			myprintf("Labeling+Centroids\t%d\n", elapsed_us(time1, get_time_stamp()));
		#endif
		/** Initialize permutation = vector used for blob -> track assignment
		 */
		int assignment[NUM_BLOBS_MAX];
		for(i = 0; i < NUM_BLOBS_MAX; i++) assignment[i] = -1;
		short unassignedCols[NUM_BLOBS_MAX], unassignedColNum;

	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
	/** Assignment heuristic.
	 * After the distance between foreseen centroid and actual centroid is bigger than UNASSIGNMENT_COST
	 * no assignment is performed.*/
	FIND_ASSIGNMENT(numTracks, numBlobs, states, centroids, UNASSIGNMENT_COST, assignment, unassignedCols, &unassignedColNum);
	#ifdef TIME_CHECK
		myprintf("Tracks\t%d\n", numTracks);
		myprintf("BBB\t%d\n", numBlobs);
	#endif


	#ifdef TIME_CHECK
		time1 = get_time_stamp();
	#endif
		/** Perform a kalman correction */
		CORRECT_ALL(numTracks, numBlobs, states, centroids, assignment);
	#ifdef TIME_CHECK
		myprintf("Correction\t%d\n", elapsed_us(time1, get_time_stamp()));
		time1 = get_time_stamp();
	#endif
		for(i = 0; i < unassignedColNum; i++) {
			states[numTracks+i].id = newId++; //assign is broken, fix it.
			states[numTracks+i].posX = centroids[unassignedCols[i]].X;
			states[numTracks+i].posY = centroids[unassignedCols[i]].Y;
			states[numTracks+i].velX = states[numTracks+i].velY = 0;
			states[numTracks+i].consecutiveInvisibleCount = 1;
			states[numTracks+i].totalVisibleCount=0;
			states[numTracks+i].age = 1;
			int j, k;
			for(j = 0; j < 4; j++)
				for(k = 0; k < 4; k++) {
					if(j == k) states[numTracks+i].cov[j][j] = INITIAL_AUTOCOV*UNIT_VALUE;
					else states[numTracks+i].cov[j][k] = 0;
				}
		}
	#ifdef TIME_CHECK
		myprintf("New tracks creation\t%d\n", elapsed_us(time1, get_time_stamp()));
	#endif
		//myprintf("New track assignment ended!");
		numTracks += unassignedColNum;

		//Send quintuple in the form:
		// <idT, x1,y1, x2, y2> for the bounding box of each blob recognized
	#ifdef TIME_CHECK
		myprintf("Total\t%d\n", get_time_stamp() - time_tot);
		for(i = 0; i < numBlobs; i++) {
			if(assignment[i] >= 0) {
				myprintf("TR:\t%d\t%d\t%d\t%d\n", stream_element, centroids[i].X, centroids[i].Y, states[assignment[i]].id);
			}
		}
	#endif
		myprintf("$");

		for(i = 0; i < numBlobs; i++) {
			if(assignment[i] >= 0) {
				myprintf(" "); //Continue reading
				myprintf("%3d", states[assignment[i]].id);
				myprintf("%3d", centroids[i].topX);
				myprintf("%3d", centroids[i].topY);
				myprintf("%3d", centroids[i].botX);
				myprintf("%3d", centroids[i].botY);
			}
		}
		myprintf("$");
		stream_element++;
	}
	return 0;
}
#endif
