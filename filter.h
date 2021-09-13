#include <stdio.h>
#include <stdlib.h>

/*
 * The function rotates the kernel matrix by a 180 degrees
 */
void rotation(int n, float kernel[n][n], float rotated_kernel[n][n]);

/*
 * All the functions apply a given filter to a pixel
 */
void apply_Smoothing_filter(float **image_in, float *pixel);
void apply_Gaussian_Blur_filter(float **image_in, float *pixel);
void apply_Sharpen_filter(float **image_in, float *pixel);
void apply_Mean_removal_filter(float **image_in, float *pixel);
void apply_Emboss_filter(float **image_in, float *pixel);
