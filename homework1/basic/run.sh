#!/bin/bash
N=$1
in_file=$2
out_file=$3
mpicc basic.c -O3 -g -o basic -lm
srun -N 2 -n 4 ./basic $N $in_file $out_file
#mpirun -np 4 ./basic $N $in_file $out_file
