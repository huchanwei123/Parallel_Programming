#!/bin/bash
file=$1
N=$2
in_file=$3
out_file=$4

rm $file
mpicc ${file}.c -O3 -g -o $file -lm
#srun -N 2 -n 4 ./$file $N $in_file $out_file
mpirun -np 4 ./$file $N $in_file $out_file
