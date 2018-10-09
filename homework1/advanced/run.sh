#!/bin/bash
N=$1
in_file=$2
out_file=$3
rm advanced
mpicc advanced.c -O3 -g -o advanced -lm
#srun -N 2 -n 4 ./advanced $N $in_file $out_file
mpirun -np 4 ./advanced $N $in_file $out_file
