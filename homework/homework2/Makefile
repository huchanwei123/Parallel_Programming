CC = gcc
MPICC = mpicc
CFLAGS = -O3 -pthread -lm -lpng -fopenmp #-DDEBUG

seq: seq.c
	 $(CC) seq.c $(CFLAGS) -o seq

mpi_static: mpi_static.c
	 $(MPICC) mpi_static.c $(CFLAGS) -o mpi_static

