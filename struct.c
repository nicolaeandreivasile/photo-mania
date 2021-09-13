#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "struct.h"

/*
 * Other functions
 */
void comments(FILE *in_file) {
        char character;
        char comment_char = '#';
        char newline = '\n';

        fscanf(in_file, "%c", &character);
        if (strncmp(&character, &comment_char, 1) == 0) {
                while(1) {
                        fscanf(in_file, "%c", &character);
                        if (strncmp(&character, &newline, 1) == 0) {
                                break;
                        }
                }
        } else {
                fseek(in_file, -1, SEEK_CUR);           
        }       
}

/*
 * PGM structure functions
 */

/* PGM structure initialization */
PGM *pgm_init(FILE *in_file) {
	// structure allocation
	PGM *pgm_struct = (PGM*) calloc(1, sizeof(PGM));
	if (!pgm_struct) {
		perror("Error: PGM initialization");
		exit(EXIT_FAILURE);
	}

	// width, height and max_value initialization
	fscanf(in_file, "%d %d\n",&(WIDTH_PGM(pgm_struct)), 
				&(HEIGHT_PGM(pgm_struct)));
	comments(in_file);
	fscanf(in_file, "%d\n", &(MAXVAL_PGM(pgm_struct)));
	comments(in_file);

	// bitmap allocation & initialization
	pgm_struct->bitmap = (uint8_t**) calloc(HEIGHT_PGM(pgm_struct) + 2, 
						sizeof(uint8_t*));
	if (!(pgm_struct->bitmap)) {
		free(pgm_struct);
		
		perror("Error: PGM bitmap row initialization");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < HEIGHT_PGM(pgm_struct) + 2; i++) {
		pgm_struct->bitmap[i] = (uint8_t*) calloc(WIDTH_PGM(pgm_struct) + 2,
							sizeof(uint8_t));
		if (!(pgm_struct->bitmap[i])) {
			for (int k = 0; k < i; k++) {
				free(pgm_struct->bitmap[k]);
			}
			free(pgm_struct->bitmap);
			free(pgm_struct);
		
			perror("Error: PGM bitmap collum initialization");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 1; i <= HEIGHT_PGM(pgm_struct); i++) {
		for (int j = 1; j <= WIDTH_PGM(pgm_struct); j++) {
			fscanf(in_file, "%c", &(pgm_struct->bitmap[i][j]));
		}
	}

	return pgm_struct;
}

/*
 * PGM piece structure initialization
 */
PGM *pgm_piece_init(uint8_t **whole_bitmap, int w_start, int w_end, 
				int h_start, int h_end, int max_value) {
	PGM *pgm_piece = (PGM*) calloc(1, sizeof(PGM));
	if (!pgm_piece) {
		perror("Error: PGM piece initialization");
		exit(EXIT_FAILURE);
	}

	int width = w_end - (w_start - 1);
	int height = h_end - (h_start - 1);
	memcpy(&(WIDTH_PGM(pgm_piece)), &width, sizeof(int));
	memcpy(&(HEIGHT_PGM(pgm_piece)), &height, sizeof(int));
	memcpy(&(MAXVAL_PGM(pgm_piece)), &max_value, sizeof(int));
	
	pgm_piece->bitmap = (uint8_t**) calloc (height + 2, sizeof(uint8_t*));
	if (!(pgm_piece->bitmap)) {
		free(pgm_piece);
		
		perror("Error: PGM piece bitmap row initialization");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < height + 2; i++) {
		pgm_piece->bitmap[i] = (uint8_t*) calloc(width + 2, sizeof(uint8_t));
		if (!(pgm_piece->bitmap[i])) {
			for (int k = 0; k < i; k++) {
				free(pgm_piece->bitmap[k]);
			}
			free(pgm_piece->bitmap);
			free(pgm_piece);

			perror("Error: PGM piece bitmap collum initialization");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < height + 2; i++) {
		for (int j = 0; j < width + 2; j++) {
			memcpy(&(pgm_piece->bitmap[i][j]), 
				&(whole_bitmap[h_start + i - 1][w_start + j - 1]), sizeof(uint8_t));
		}
	}

	return pgm_piece;
}

/* PGM structure removal */
void pgm_free(PGM *pgm_struct) {
	for (int i = 0; i < HEIGHT_PGM(pgm_struct) + 2; i++) {
		free(pgm_struct->bitmap[i]);
	}
	free(pgm_struct->bitmap);
	free(pgm_struct);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \\

/*
 * PNM structure functions
 */

/* PNM structure initialization */
PNM *pnm_init(FILE *in_file) {
	PNM *pnm_struct = (PNM*) calloc(1, sizeof(PNM));
	if (!pnm_struct) {
		perror("Error: PNM initialization");
		exit(EXIT_FAILURE);
	}

	// width, height and max_value initialization
	fscanf(in_file, "%d %d\n",&(WIDTH_PNM(pnm_struct)), 
				&(HEIGHT_PNM(pnm_struct)));
	comments(in_file);
	fscanf(in_file, "%d\n", &(MAXVAL_PGM(pnm_struct)));
	comments(in_file);

	// bitmap allocation & initialization
	pnm_struct->bitmap = (RGB**) calloc(HEIGHT_PNM(pnm_struct) + 2, 
						sizeof(RGB*));
	if (!(pnm_struct->bitmap)) {
		free(pnm_struct);
		
		perror("Error: PNM bitmap row initialization");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < HEIGHT_PNM(pnm_struct) + 2; i++) {
		pnm_struct->bitmap[i] = (RGB*) calloc(WIDTH_PNM(pnm_struct) + 2,
							sizeof(RGB));
		if (!(pnm_struct->bitmap[i])) {
			for (int k = 0; k < i; k++) {
				free(pnm_struct->bitmap[k]);
			}
			free(pnm_struct->bitmap);
			free(pnm_struct);
		
			perror("Error: PNM bitmap collum initialization");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 1; i <= HEIGHT_PNM(pnm_struct); i++) {
		for (int j = 1; j <= WIDTH_PNM(pnm_struct); j++) {
			fscanf(in_file, "%c", &((pnm_struct->bitmap[i][j]).red));
			fscanf(in_file, "%c", &((pnm_struct->bitmap[i][j]).green));
			fscanf(in_file, "%c", &((pnm_struct->bitmap[i][j]).blue));
		}
	}

	return pnm_struct;
}

/*
 * PNM structure piece initialization
 */
PNM *pnm_piece_init(RGB **whole_bitmap, int w_start, int w_end, 
			int h_start, int h_end, int max_value) {
	PNM *pnm_piece = (PNM*) calloc(1, sizeof(PNM));
	if (!pnm_piece) {
		perror("Error: PNM piece initialization");
		exit(EXIT_FAILURE);
	}

	int width = w_end - (w_start - 1);
	int height = h_end - (h_start - 1);
	memcpy(&(WIDTH_PNM(pnm_piece)), &width, sizeof(int));
	memcpy(&(HEIGHT_PNM(pnm_piece)), &height, sizeof(int));
	memcpy(&(MAXVAL_PNM(pnm_piece)), &max_value, sizeof(int));

	pnm_piece->bitmap = (RGB**) calloc (height + 2, sizeof(RGB*));
	if (!(pnm_piece->bitmap)) {
		free(pnm_piece);
		
		perror("Error: PNM piece bitmap row initialization");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < height + 2; i++) {
		pnm_piece->bitmap[i] = (RGB*) calloc(width + 2, sizeof(RGB));
		if (!(pnm_piece->bitmap[i])) {
			for (int k = 0; k < i; k++) {
				free(pnm_piece->bitmap[k]);
			}
			free(pnm_piece->bitmap);
			free(pnm_piece);

			perror("Error: PNM piece bitmap collum initialization");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < height + 2; i++) {
		for (int j = 0; j < width + 2; j++) {
			memcpy(&(pnm_piece->bitmap[i][j]), 
				&(whole_bitmap[h_start + i - 1][w_start + j - 1]), sizeof(RGB));
		}
	}

	return pnm_piece;
}

/* PNM structure removal */
void pnm_free(PNM *pnm_struct) {
	for (int i = 0; i < HEIGHT_PNM(pnm_struct) + 2; i++) {
		free(pnm_struct->bitmap[i]);
	}
	free(pnm_struct->bitmap);
	free(pnm_struct);

}
