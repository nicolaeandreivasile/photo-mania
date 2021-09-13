all: build

build:
	mpicc -o photo_mania photo_mania.c struct.c filter.c -lm

clean:
	rm -f tema3
