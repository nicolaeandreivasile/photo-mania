#include <stdio.h>
#include <stdlib.h>
#include "filter.h"

/*
 * Kernel rotation by 180 degrees
 */
void rotation(int n, float kernel[n][n], float rotated_kernel[n][n]) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			rotated_kernel[i][j] = kernel[n - 1 - i][n - 1 - j];
		}
	} 
}

/*
 * Smoothing filter
 */
void apply_Smoothing_filter(float **image_in, float *pixel) {
	// filter kernel
	float K[3][3] = {{1, 1, 1}, 
			 {1, 1, 1}, 
			 {1, 1, 1}};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			K[i][j] *= (float) 1 / 9;
		}
	}
	
	//rotated filter kernel
	float rotated_K[3][3];
	rotation(3, K, rotated_K);

	// computing pixel
	*pixel = 0;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			*pixel += image_in[i][j] * rotated_K[i][j];
		}
	}	
}

/*
 * Gaussian Blur filter
 */
void apply_Gaussian_Blur_filter(float **image_in, float *pixel) {
	// filter kernel
	float K[3][3] = {{1, 2, 1}, 
			 {2, 4, 2}, 
			 {1, 2, 1}};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			K[i][j] *= (float) 1 / 16;
		}
	}

	//rotated filter kernel
	float rotated_K[3][3];
	rotation(3, K, rotated_K);

	// computing pixel
	*pixel = 0;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			*pixel += image_in[i][j] * rotated_K[i][j];
		}
	}
}

/*
 * Sharpen filter
 */
void apply_Sharpen_filter(float **image_in, float *pixel) {
	// filter kernel
	float K[3][3] = {{0, -2, 0}, 
			 {-2, 11, -2}, 
			 {0, -2, 0}};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			K[i][j] *= (float) 1 / 3;
		}
	}

	//rotated filter kernel
	float rotated_K[3][3];
	rotation(3, K, rotated_K);

	// computing pixel
	*pixel = 0;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			*pixel += image_in[i][j] * rotated_K[i][j];
		}
	}
}

/*
 * Mean removal filter
 */
void apply_Mean_removal_filter(float **image_in, float *pixel) {
	// filter kernel
	float K[3][3] = {{-1, -1, -1}, 
			 {-1, 9, -1}, 
			 {-1, -1, -1}};

	//rotated filter kernel
	float rotated_K[3][3];
	rotation(3, K, rotated_K);

	// computing pixel
	*pixel = 0;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			*pixel += image_in[i][j] * rotated_K[i][j];
		}
	}
}

/*
 * Emboss filter
 */
void apply_Emboss_filter(float **image_in, float *pixel) {
	// filter kernel
	float K[3][3] = {{0, 1, 0}, 
			 {0, 0, 0}, 
			 {0, -1, 0}};

	//rotated filter kernel
	float rotated_K[3][3];
	rotation(3, K, rotated_K);

	// computing pixel
	*pixel = 0;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			*pixel += image_in[i][j] * rotated_K[i][j];
		}
	}
}
