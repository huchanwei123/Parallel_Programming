#!/bin/bash
N=$1
in_file=$2
out_file=$3
mpicxx -std=c++11 basic.cc -O3 -o basic -lm
mpirun -np 4 ./basic $N $in_file $out_file
#g++ basic.cc -O3 -o basic -lm
#./basic $in_file
