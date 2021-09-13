#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "filter.h"
#include "struct.h"

#define MAX_COMMENTS	1000
#define MAX_FILTERNAME	10
#define MASTER		0

/*
 * Make an array with all the given filters
 */
char **make_filter_array(int argc, char *argv[]) {
	char **filters = (char**) calloc(argc - 3, sizeof(char*));
	for (int i = 3; i < argc; i++) {
		filters[i - 3] = (char*) calloc(MAX_FILTERNAME, sizeof(char));
		memcpy(filters[i - 3], argv[i], strnlen(argv[i], MAX_FILTERNAME));
	}

	return filters;
}

/*
 * Apply a given filter to an image
 */
void apply_filter(uint8_t** bitmap, int width, int height, int max_value, 
			char *filter) {
	/*
	 * Initialize the matrix of the image after apllying the filter
	 * Initialize the matrix of each square to be computed
	 */
	uint8_t **computed_image = (uint8_t**) calloc(height + 2, sizeof(uint8_t*));
	for (int i = 0; i < height + 2; i++) {
		computed_image[i] = (uint8_t*) calloc(width + 2, sizeof(uint8_t));
	}

	float **square = (float**) calloc(3, sizeof(float*));
	for (int i = 0; i < 3; i++) {
		square[i] = (float*) calloc(3, sizeof(float));
	}

	/* Apply the given filter to each square */
	for (int i = 1; i <= height; i++) {
		for (int j = 1; j <= width; j++) {
			for (int aux_i = 0; aux_i < 3; aux_i++) {
				for (int aux_j = 0; aux_j < 3; aux_j++) {
					square[aux_i][aux_j] = (float) bitmap[i + aux_i - 1][j + aux_j - 1];
				}
			}	

			float pixel;
			uint8_t computed_pixel;
			
			/* Choose the right filter */	
			if (strncmp(filter, "smooth", MAX_FILTERNAME) == 0) {
				apply_Smoothing_filter(square, &pixel);
			} else if (strncmp(filter, "blur", MAX_FILTERNAME) == 0) {
				apply_Gaussian_Blur_filter(square, &pixel);
			} else if (strncmp(filter, "sharpen", MAX_FILTERNAME) == 0) {
				apply_Sharpen_filter(square, &pixel);
			} else if (strncmp(filter, "mean", MAX_FILTERNAME) == 0) {
				apply_Mean_removal_filter(square, &pixel);
			} else if (strncmp(filter, "emboss", MAX_FILTERNAME) == 0) {
				apply_Emboss_filter(square, &pixel);
			} else {
				perror("USED FILTERS: smooth, blur, sharpen, mean, emboss");
				exit(EXIT_FAILURE);
			}

			/* Clamping the result */
			if (pixel < 0) {
				computed_pixel = 0;
			} else if (pixel > max_value) {
				computed_pixel = max_value;
			} else {
				computed_pixel = (uint8_t) pixel;
			}

			memcpy(&(computed_image[i][j]), &computed_pixel, sizeof(uint8_t));	
		}
	}

	/* Save the computed image into bitmap buffer */
	for (int i = 1; i <= height; i++) {
		for (int j = 1; j <= width; j++) {
			memcpy(&(bitmap[i][j]), &(computed_image[i][j]), sizeof(uint8_t));
		}
	}

	/* Free allocated memory */
	for (int i = 0; i < height; i++) {
		free(computed_image[i]);
	}
	free(computed_image);

	for (int i = 0; i < 3; i++) {
		free(square[i]);
	}
	free(square);
}

/*
 * Write the image after applying filters
 */
void write_image(FILE *out_file, PGM *PGM_image, PNM *PNM_image) {
	if (PGM_image) {
	/* Write PGM image */
		fprintf(out_file, "%s\n", "P5");
		fprintf(out_file, "%d %d\n", WIDTH_PGM(PGM_image), 
						HEIGHT_PGM(PGM_image));
		fprintf(out_file, "%d\n", MAXVAL_PGM(PGM_image));

		for (int i = 1; i <= HEIGHT_PGM(PGM_image); i++) {
			for (int j = 1; j <= WIDTH_PGM(PGM_image); j++) {
				fprintf(out_file, "%c", PGM_image->bitmap[i][j]);
			}
		}
	} else if (PNM_image) {
	/* Write PNM image */
		fprintf(out_file, "%s\n", "P6");
		fprintf(out_file, "%d %d\n", WIDTH_PNM(PNM_image), 
						HEIGHT_PNM(PNM_image));
		fprintf(out_file, "%d\n", MAXVAL_PNM(PNM_image));

		for (int i = 1; i <= HEIGHT_PNM(PNM_image); i++) {
			for (int j = 1; j <= WIDTH_PNM(PNM_image); j++) {
				fprintf(out_file, "%c", PNM_image->bitmap[i][j].red);
				fprintf(out_file, "%c", PNM_image->bitmap[i][j].green);
				fprintf(out_file, "%c", PNM_image->bitmap[i][j].blue);
			}
		}
	}
}

/*
 * Main function
 */
int main(int argc, char *argv[]) {
	int rank;
	int no_processes;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &no_processes);

	if (rank == MASTER) {		
	/* Process is MASTER */
		if (argc < 2) {
			fprintf(stdout, "USAGE: mpirun -n no_processes ./tema3 image_in image_out filter1 filter2 ... filterN");
			exit(EXIT_SUCCESS);
		}

		/* Open input and output files */
		char *in_filename = argv[1];
		FILE *in_file = fopen(in_filename, "r");
		if (!in_file) {
			perror("Error: Input file not found");
			exit(EXIT_FAILURE);
		}
		fseek(in_file, 0, SEEK_SET);
	
		char *out_filename = argv[2];
		FILE *out_file = fopen (out_filename, "w");
		fseek(out_file, 0, SEEK_SET);

		/* Save filters into array */
		char **filters = NULL;
		if (argc > 2) {
			filters = make_filter_array(argc, argv);
		}

		/* Read type of image */
		char *type = (char*) calloc(2, sizeof(char));
		char *PGM_type = (char*) calloc(2, sizeof(char));
		memcpy(PGM_type, "P5", 2);
		char *PNM_type = (char*) calloc(2, sizeof(char));
		memcpy(PNM_type, "P6", 2);

		fscanf(in_file, "%s\n", type);
		comments(in_file);		// ignore comments
		
		if (strncmp(type, PGM_type, 2) == 0) {
		/* PGM image */	
			PGM *image_struct = pgm_init(in_file);
			uint8_t **computed_bitmap = (uint8_t**) calloc(HEIGHT_PGM(image_struct) + 2, 
									sizeof(uint8_t*));
			for (int i = 0; i < HEIGHT_PGM(image_struct) + 2; i++) {
				computed_bitmap[i] = (uint8_t*) calloc(WIDTH_PGM(image_struct) + 2, 
											sizeof(uint8_t));
			}

			for (int h = 0; h < argc - 3; h++) {
			/* For each filter */
				for (int i = 0; i < no_processes; i++) {
				/* For each running process */
					/* Compute bounds */
					int width_start = 1;
					int width_end = WIDTH_PGM(image_struct);
					int height_start = i * ceil((double) HEIGHT_PGM(image_struct) / no_processes) + 1;
					int height_end = fmin(HEIGHT_PGM(image_struct), 
							(i + 1) * ceil((double) HEIGHT_PGM(image_struct) / no_processes));

					/* Make image piece */
					PGM *image_piece = pgm_piece_init(image_struct->bitmap, 
							width_start, width_end, 
							height_start, height_end,
							MAXVAL_PGM(image_struct));	
				
					if (i == MASTER) {
					/* Process is MASTER */
						/* Apply filter to image piece */
						apply_filter(image_piece->bitmap, WIDTH_PGM(image_piece), 
							HEIGHT_PGM(image_piece), MAXVAL_PGM(image_piece), filters[h]);
						
						/* Save computed image piece into buffer */
						for (int k = height_start; k <= height_end; k++) {
							for (int j = width_start; j <= width_end; j++) {
								memcpy(&(computed_bitmap[k][j]), 
								&(image_piece->bitmap[k - (height_start - 1)][j - (width_start - 1)]), 
								sizeof(uint8_t));
							}
						}
					} else {
					/* Process is not MASTER */
						/* Send filter name */
						MPI_Send(filters[h], MAX_FILTERNAME, MPI_CHAR, i, 0, MPI_COMM_WORLD);
						
						/* Send image type */
				 		MPI_Send(PGM_type, 2, MPI_CHAR, i, 0, MPI_COMM_WORLD);

						/* Send ints */
						int ints_buffer_size = 3;
						int *ints_buffer = (int*) calloc(ints_buffer_size, sizeof(int));
						memcpy(ints_buffer, &(WIDTH_PGM(image_piece)), sizeof(int));
						memcpy(ints_buffer + 1, &(HEIGHT_PGM(image_piece)), 
									sizeof(int));
						memcpy(ints_buffer + 2, &(MAXVAL_PGM(image_piece)), 
									sizeof(int));
						MPI_Send(ints_buffer, ints_buffer_size, MPI_INT, i, 0, MPI_COMM_WORLD);

						/* Send bitmap */
						int bitmap_buffer_size = (HEIGHT_PGM(image_piece) + 2) * (WIDTH_PGM(image_piece) + 2);
						uint8_t *bitmap_buffer = (uint8_t*) calloc(bitmap_buffer_size, sizeof(uint8_t));
						for (int k = 0; k < HEIGHT_PGM(image_piece) + 2; k++) {
							for (int j = 0; j < WIDTH_PGM(image_piece) + 2; j++) {
								int offset = (k * (WIDTH_PGM(image_piece) + 2)) + j;
								memcpy(bitmap_buffer + offset, &(image_piece->bitmap[k][j]), 
											sizeof(uint8_t));
							}
						}

						MPI_Send(bitmap_buffer, bitmap_buffer_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
					
						/* Free allocated memory */
						free(ints_buffer);
						free(bitmap_buffer);
					}

					/* Free allocated memory */
					pgm_free(image_piece);
				}	
			
			
				for (int i = 1; i < no_processes; i++) {
				/* For NON-MASTER running processes */
					/* Compute bounds */
					int width_start = 1;
					int width_end = WIDTH_PGM(image_struct);
					int height_start = i * ceil((double) HEIGHT_PGM(image_struct) / no_processes) + 1;
					int height_end = fmin(HEIGHT_PGM(image_struct), 
								(i + 1) * ceil((double) HEIGHT_PGM(image_struct) / no_processes));
					
					/* Receive computed image piece */
					int width = width_end - (width_start - 1);
					int height = height_end - (height_start - 1);
	
					int bitmap_buffer_size = (height + 2) * (width + 2);
					uint8_t *bitmap_buffer = (uint8_t*) calloc(bitmap_buffer_size, sizeof(uint8_t));
					MPI_Recv(bitmap_buffer, bitmap_buffer_size, MPI_CHAR, i, 0, MPI_COMM_WORLD, 
								MPI_STATUS_IGNORE);
	
					/* Save computed image piece into buffer */
					for (int k = height_start; k <= height_end; k++) {
						for (int j = width_start; j <= width_end; j++) {
							int offset = ((k - (height_start - 1)) 
									* (width_end - (width_start - 1) + 2)) 
									+ (j - (width_start - 1));
							memcpy(&(computed_bitmap[k][j]), bitmap_buffer + offset, sizeof(uint8_t));
						}
					}	
					
					/* Free allocated memory */
					free(bitmap_buffer);		
				}
	
				/* Save computed buffer(image) into image structure */
				for (int i = 0; i < HEIGHT_PGM(image_struct) + 2; i++) {
					for (int j = 0; j < WIDTH_PGM(image_struct) + 2; j++) {
						memcpy(&(image_struct->bitmap[i][j]), &(computed_bitmap[i][j]), sizeof(uint8_t));
					}
				}
			}

			/* Write image */
			write_image(out_file, image_struct, NULL);
			
			/* Free allocated memory */
			for (int i = 0; i < HEIGHT_PGM(image_struct) + 2; i++) {
				free(computed_bitmap[i]);
			}
			free(computed_bitmap);
			pgm_free(image_struct);
		} else if (strncmp(type, PNM_type, 2) == 0) {
		/* PNM image */
			PNM *image_struct = pnm_init(in_file);
			RGB **computed_bitmap = (RGB**) calloc(HEIGHT_PNM(image_struct) + 2, sizeof(RGB*));
			for (int i = 0; i < HEIGHT_PNM(image_struct) + 2; i++) {
				computed_bitmap[i] = (RGB*) calloc(WIDTH_PNM(image_struct) + 2, sizeof(RGB));
			}

			for (int h = 0; h < argc - 3; h++) {
			/* For each filter */
				for (int i = 0; i < no_processes; i++) {
				/* For each running process */
					/* Compute bounds */
					int width_start = 1;
					int width_end = WIDTH_PNM(image_struct);
					int height_start = i * ceil((double) HEIGHT_PNM(image_struct) / no_processes) + 1;
					int height_end = fmin(HEIGHT_PNM(image_struct), 
							(i + 1) * ceil((double) HEIGHT_PNM(image_struct) / no_processes));
					
					/* Make image piece */
					PNM *image_piece = pnm_piece_init(image_struct->bitmap, 
							width_start, width_end, 
							height_start, height_end,
							MAXVAL_PNM(image_struct));	
				
					if (i == MASTER) {
					/* Process is MASTER */
						/* Apply filter to image piece */
						uint8_t **bitmap_red = (uint8_t**) calloc(HEIGHT_PNM(image_piece) + 2, 
												sizeof(uint8_t*));
						uint8_t **bitmap_green = (uint8_t**) calloc(HEIGHT_PNM(image_piece) + 2, 
												sizeof(uint8_t*));
						uint8_t **bitmap_blue = (uint8_t**) calloc(HEIGHT_PNM(image_piece) + 2, 
												sizeof(uint8_t*));
						for (int k = 0; k < HEIGHT_PNM(image_piece) + 2; k++) {
							bitmap_red[k] = (uint8_t*) calloc(WIDTH_PNM(image_piece) + 2, 
												sizeof(uint8_t));
							bitmap_green[k] = (uint8_t*) calloc(WIDTH_PNM(image_piece) + 2, 
												sizeof(uint8_t));
							bitmap_blue[k] = (uint8_t*) calloc(WIDTH_PNM(image_piece) + 2, 
												sizeof(uint8_t));
							for (int j = 0; j < WIDTH_PNM(image_piece) + 2; j++) {
								memcpy(&(bitmap_red[k][j]), &(image_piece->bitmap[k][j].red), 
													sizeof(uint8_t));
								memcpy(&(bitmap_green[k][j]), &(image_piece->bitmap[k][j].green), 
													sizeof(uint8_t));
								memcpy(&(bitmap_blue[k][j]), &(image_piece->bitmap[k][j].blue), 
													sizeof(uint8_t));
							}
						}

						apply_filter(bitmap_red, WIDTH_PNM(image_piece), HEIGHT_PNM(image_piece), 
								MAXVAL_PNM(image_piece), filters[h]);
						apply_filter(bitmap_green, WIDTH_PNM(image_piece), HEIGHT_PNM(image_piece), 
								MAXVAL_PNM(image_piece), filters[h]);
						apply_filter(bitmap_blue, WIDTH_PNM(image_piece), HEIGHT_PNM(image_piece), 
								MAXVAL_PNM(image_piece), filters[h]);

						/* Save computed image piece into buffer */
						for (int k = height_start; k <= height_end; k++) {
							for (int j = width_start; j <= width_end; j++) {
								memcpy(&(computed_bitmap[k][j].red), 
									&(bitmap_red[k - (height_start - 1)][j - (width_start - 1)]), 
									sizeof(uint8_t));
								memcpy(&(computed_bitmap[k][j].green), 
									&(bitmap_green[k - (height_start - 1)][j - (width_start - 1)]), 
									sizeof(uint8_t));
								memcpy(&(computed_bitmap[k][j].blue), 
									&(bitmap_blue[k - (height_start - 1)][j - (width_start - 1)]), 
									sizeof(uint8_t));
							}
						}
					} else {
					/* Process is not MASTER */
						/* Send filter name */
						MPI_Send(filters[h], MAX_FILTERNAME, MPI_CHAR, i, 0, MPI_COMM_WORLD);
						
						/* Send image type */
				 		MPI_Send(PNM_type, 2, MPI_CHAR, i, 0, MPI_COMM_WORLD);

						/* Send ints */
						int ints_buffer_size = 3;
						int *ints_buffer = (int*) calloc(ints_buffer_size, sizeof(int));
						memcpy(ints_buffer, &(WIDTH_PNM(image_piece)), sizeof(int));
						memcpy(ints_buffer + 1, &(HEIGHT_PNM(image_piece)), 
									sizeof(int));
						memcpy(ints_buffer + 2, &(MAXVAL_PNM(image_piece)), 
									sizeof(int));
						MPI_Send(ints_buffer, ints_buffer_size, MPI_INT, i, 0, MPI_COMM_WORLD);

						/* Send bitmap */
						int bitmap_buffer_size = (HEIGHT_PNM(image_piece) + 2) * (WIDTH_PNM(image_piece) + 2);
						uint8_t *bitmap_buffer = (uint8_t*) calloc(3 * bitmap_buffer_size, sizeof(uint8_t));
							
						for (int k = 0; k < HEIGHT_PNM(image_piece) + 2; k++) {
							for (int j = 0; j < WIDTH_PNM(image_piece) + 2; j++) {
								int offset = (k * (WIDTH_PNM(image_piece) + 2)) + j;
								memcpy(bitmap_buffer + offset, 
										&(image_piece->bitmap[k][j].red), sizeof(uint8_t));
								memcpy(bitmap_buffer + bitmap_buffer_size + offset, 
										&(image_piece->bitmap[k][j].green), sizeof(uint8_t));
								memcpy(bitmap_buffer + (2 * bitmap_buffer_size) + offset, 
										&(image_piece->bitmap[k][j].blue), sizeof(uint8_t));
							}
						}
						
						MPI_Send(bitmap_buffer, (3 * bitmap_buffer_size), MPI_CHAR, i, 0, MPI_COMM_WORLD);
						
						/* Free allocated memory */
						free(ints_buffer);
						free(bitmap_buffer);
					}

					/* Free allocated memory */
					pnm_free(image_piece);
				}	
			
			
				for (int i = 1; i < no_processes; i++) {
				/* For NON-MASTER running processes */
					/* Compute bounds */
					int width_start = 1;
					int width_end = WIDTH_PGM(image_struct);
					int height_start = i * ceil((double) HEIGHT_PGM(image_struct) / no_processes) + 1;
					int height_end = fmin(HEIGHT_PGM(image_struct), 
								(i + 1) * ceil((double) HEIGHT_PGM(image_struct) / no_processes));
					
					/* Receive computer image piece */
					int width = width_end - (width_start - 1);
					int height = height_end - (height_start - 1);
	
					int bitmap_buffer_size = (height + 2) * (width + 2);
					uint8_t *bitmap_buffer = (uint8_t*) calloc(3 * bitmap_buffer_size, sizeof(uint8_t));
					MPI_Recv(bitmap_buffer, (3 * bitmap_buffer_size), MPI_CHAR, i, 0, MPI_COMM_WORLD, 
									MPI_STATUS_IGNORE);
					
					/* Save computed image piece into buffer */
					for (int k = height_start; k <= height_end; k++) {
						for (int j = width_start; j <= width_end; j++) {
							int offset = ((k - (height_start - 1)) 
									* (width_end - (width_start - 1) + 2)) 
									+ (j - (width_start - 1));
							memcpy(&(computed_bitmap[k][j].red), 
									bitmap_buffer + offset, sizeof(uint8_t));
							memcpy(&(computed_bitmap[k][j].green), 
									bitmap_buffer + bitmap_buffer_size + offset, sizeof(uint8_t));
							memcpy(&(computed_bitmap[k][j].blue), 
									bitmap_buffer + (2 * bitmap_buffer_size) + offset, sizeof(uint8_t));
						}
					}	
					
					/* Free allocated memory */
					free(bitmap_buffer);
				}
	
				/* Save computed buffer(image) into image structure */
				for (int i = 0; i < HEIGHT_PNM(image_struct) + 2; i++) {
					for (int j = 0; j < WIDTH_PNM(image_struct) + 2; j++) {
						memcpy(&(image_struct->bitmap[i][j]), &(computed_bitmap[i][j]), sizeof(RGB));
					}
				}
			}

			/* Write image */
			write_image(out_file, NULL, image_struct);
			
			/* Free allocated memory */
			for (int i = 0; i < HEIGHT_PNM(image_struct) + 2; i++) {
				free(computed_bitmap[i]);
			}
			free(computed_bitmap);
			pnm_free(image_struct);
		}

		/* Free allocated memory */
		free(type);
		free(PGM_type);
		free(PNM_type);
		for (int i = 0; i < argc - 3; i++) {
			free(filters[i]);
		}
		free(filters);
	} else {
	/* Process is NON-MASTER */
		/* Initialize buffers */
		char *filter = (char*) calloc(MAX_FILTERNAME, sizeof(char));

		char *type = (char*) calloc(2, sizeof(char));	
		char *PGM_type = (char*) calloc(2, sizeof(char));
		memcpy(PGM_type, "P5", 2);
		char *PNM_type = (char*) calloc(2, sizeof(char));
		memcpy(PNM_type, "P6", 2);

		int ints_buffer_size = 3;
		int *ints_buffer = (int*) calloc(ints_buffer_size, sizeof(int));
			
		for (int h = 0; h < argc - 3; h++) {	
		/* For each filter */
			/* Receive filter, type and ints of the image */	
			MPI_Recv(filter, MAX_FILTERNAME, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(type, 2, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(ints_buffer, ints_buffer_size, MPI_INT, MASTER, 0, MPI_COMM_WORLD, 
						MPI_STATUS_IGNORE);
			
			int width, height, max_value;
			memcpy(&width, ints_buffer, sizeof(int));
			memcpy(&height, ints_buffer + 1, sizeof(int));
			memcpy(&max_value, ints_buffer + 2, sizeof(int));
		
			if (strncmp(type, PGM_type, 2) == 0) {
			/* PGM image */
				/* Receive image piece */
				int bitmap_buffer_size = (height + 2) * (width + 2);
				uint8_t *bitmap_buffer = (uint8_t*) calloc(bitmap_buffer_size, sizeof(uint8_t));
				MPI_Recv(bitmap_buffer, bitmap_buffer_size, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD, 
							MPI_STATUS_IGNORE);
	
				uint8_t **bitmap = (uint8_t**) calloc(height + 2, sizeof(uint8_t*));
				for (int i = 0; i < height + 2; i++) {
					bitmap[i] = (uint8_t*) calloc(width + 2, sizeof(uint8_t));
					for (int j = 0; j < width + 2; j++) {
						int offset = (i * (width + 2)) + j;
						memcpy(&(bitmap[i][j]), bitmap_buffer + offset, sizeof(uint8_t));
					}
				}
				
				/* Apply filter to received image piece */ 
				apply_filter(bitmap, width, height, max_value, filter);
							
				/* Send computed image piece */
				for (int i = 0; i < height + 2; i++) {
					for (int j = 0; j < width + 2; j++) {
						int offset = (i * (width + 2)) + j;
						memcpy(bitmap_buffer + offset, &(bitmap[i][j]), sizeof(uint8_t));
					}
				}
						
				MPI_Send(bitmap_buffer, bitmap_buffer_size, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD);
	
				/* free allocated memory */
				free(bitmap_buffer);
				for (int i = 0; i < height + 2; i++) {
					free(bitmap[i]);
				}

				free(bitmap);
			} else if (strncmp(type, PNM_type, 2) == 0){
			/* PNM image */
				/* Receive image piece */
				int bitmap_buffer_size = (height + 2) * (width + 2);
				uint8_t *bitmap_buffer = (uint8_t*) calloc(3 * bitmap_buffer_size, sizeof(uint8_t));
				MPI_Recv(bitmap_buffer, 3 * bitmap_buffer_size, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD, 
							MPI_STATUS_IGNORE);
				
				uint8_t **bitmap_red = (uint8_t**) calloc(height + 2, sizeof(uint8_t*));
				uint8_t **bitmap_green = (uint8_t**) calloc(height + 2, sizeof(uint8_t*));
				uint8_t **bitmap_blue = (uint8_t**) calloc(height + 2, sizeof(uint8_t*));
				for (int i = 0; i < height + 2; i++) {
					bitmap_red[i] = (uint8_t*) calloc(width + 2, sizeof(uint8_t));
					bitmap_green[i] = (uint8_t*) calloc(width + 2, sizeof(uint8_t));
					bitmap_blue[i] = (uint8_t*) calloc(width + 2, sizeof(uint8_t));
					for (int j = 0; j < width + 2; j++) {
						int offset = (i * (width + 2)) + j;
						memcpy(&(bitmap_red[i][j]), bitmap_buffer + offset, sizeof(uint8_t));
						memcpy(&(bitmap_green[i][j]), bitmap_buffer + bitmap_buffer_size + offset, 
									sizeof(uint8_t));
						memcpy(&(bitmap_blue[i][j]), bitmap_buffer + (2 * bitmap_buffer_size) + offset, 
									sizeof(uint8_t));
					}
				}
	
				/* Apply filter to each channel of the received image piece */
				apply_filter(bitmap_red, width, height, max_value, filter);
				apply_filter(bitmap_green, width, height, max_value, filter);
				apply_filter(bitmap_blue, width, height, max_value, filter);

				/* Send computed image piece */
				for (int i = 0; i < height + 2; i++) {
					for (int j = 0; j < width + 2; j++) {
						int offset = (i * (width + 2)) + j;
						memcpy(bitmap_buffer + offset, &(bitmap_red[i][j]), sizeof(uint8_t));
						memcpy(bitmap_buffer + bitmap_buffer_size + offset, &(bitmap_green[i][j]), 
									sizeof(uint8_t));
						memcpy(bitmap_buffer + (2 * bitmap_buffer_size) + offset, &(bitmap_blue[i][j]), 
									sizeof(uint8_t));
					}
				}
				
				MPI_Send(bitmap_buffer, 3 * bitmap_buffer_size, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD);
				
				/* Free allocated memory */
				free(bitmap_buffer);
				for (int i = 0; i < height + 2; i++) {
					free(bitmap_red[i]);
					free(bitmap_green[i]);
					free(bitmap_blue[i]);
				}
				free(bitmap_red);
				free(bitmap_green);
				free(bitmap_blue);
			}	
		}
		/* Free allocated memory */	
		free(filter);
		free(PGM_type);
		free(PNM_type);
		free(type);
		free(ints_buffer);
	}

	MPI_Finalize();
	return 0;
}
