#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
 * PGM structure
 */
typedef struct pgm {
	int width;
	int height;
	int max_value;
	uint8_t **bitmap;
} PGM;

#define WIDTH_PGM(p)		((PGM*) p)->width
#define HEIGHT_PGM(p)		((PGM*) p)->height
#define MAXVAL_PGM(p)		((PGM*) p)->max_value

/*
 * PGM structure functions
 */
PGM *pgm_init(FILE *in_file);					// structure iniitialization
PGM *pgm_piece_init(uint8_t **whole_bitmap, int w_start, int w_end, 
			int h_start, int h_end, int max_value);	// structure piece initialization
void pgm_free(PGM *pgm_struct);					// structure removal

/* 
 * RGB Structure
 */
typedef struct rgb {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} RGB;

/*
 * PNM Structure
 */
typedef struct pnm {
	int width;
	int height;
	int max_value;
	RGB **bitmap;
} PNM;

#define WIDTH_PNM(p)		((PNM*) p)->width
#define HEIGHT_PNM(p)		((PNM*) p)->height
#define MAXVAL_PNM(p)		((PNM*) p)->max_value

/*
 * PNM Structure functions
 */
PNM *pnm_init(FILE *in_file);					// structure initialization	 
PNM *pnm_piece_init(RGB **whole_bitmap, int w_start, int w_end, 
			int h_start, int h_end, int max_value);	// structure piece initialization
void pnm_free(PNM *pnm_struct);					// structure removal

/*
 * Other functions
 */
void comments(FILE *in_file);		//ignores the comments from the file
