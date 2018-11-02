CC = gcc
MPICC = mpicc
CFLAGS = -O3 -pthread -lm -lpng -fopenmp #-DDEBUG

mpi_static: mpi_static.c
	 $(MPICC) mpi_static.c $(CFLAGS) -o mpi_static

mpi_dynamic: mpi_dynamic.c
	 $(MPICC) mpi_dynamic.c $(CFLAGS) -o mpi_dynamic

hybrid: hybrid.c 
	$(MPICC) hybrid.c $(CFLAGS) -o hybrid
