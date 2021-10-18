# Photo Mania

## Description
	Implementation of a distributed algorithm that applies filters to a PGM/PNM image using MPI.
	The application reads the header of the image and then the bitmap block, that is aterwards framed with 0s. For each filter the bitmap block of the image is separated into smaller pieces, the number of pieces being equal to the number of processes, and sent to its corresponding process. Each process, after recieving the bimap piece, applies the current filter and sends the computed piece to the main procces where the image is being put togheter. It creates a new image if it is not existent, and writes the same header of the input image and the unframed computed bitmap block, resulting the image with the filters applied.

## Filters	
1. Smoothing filter: ameliorates the differences in images.
2. Approximative Gaussian Blur filter: reduces the background noise in images.
3. Sharpen filter: accentuates the image details.
4. Mean filter: same as sharpen, diagonal pixels are used also. 
5. Emboss: used for detecting edges.
	
## Usage
COMPILE: make build
USAGE: mpirun -n no_processes ./tema3 image_in image_out filter1 
		filter2 ... filterN
CLEANING: make clean

## Performance study
Scalability for the distributed algorithm:
- PGM pictures:
	Picture: rorschach.pgm (3853 x 2000)
	Filters: bssembssem (blur smooth sharpen emboss mean blur smooth sharpen emboss mean)
	Average time: (arithmetic median of values from 10 runs)
		- 1 process:		13.6 seconds
		- 2 processes:		14.8 seconds
		- 3 processes:		10.7 seconds
		- 4 processes:		8.6 seconds

- PNM pictures:
	Picture: landcape.pnm (3840 x 2160)
	Filters: bssembssem (blur smooth sharpen emboss mean blur smooth sharpen emboss mean)
	Average time: (arithmetic median of values from 10 runs)
		- 1 process:		43.8 seconds
		- 2 processes:		46.4 seconds
		- 3 processes:		34.8 seconds
		- 4 processes:		26.5 seconds
