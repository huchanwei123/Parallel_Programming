#!/bin/bash
N=$1
in_file=$2
out_file=$3
mpicxx -std=c++11 basic.cc -O3 -o basic -lm
#srun -n 4 ./basic $N $in_file $out_file
mpirun -np 4 ./basic $N $in_file $out_file
