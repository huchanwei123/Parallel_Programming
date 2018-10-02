#!/bin/bash
N=$1
mpicc pi.c -O3 -o pi -lm
mpirun -np 4 ./pi $N
