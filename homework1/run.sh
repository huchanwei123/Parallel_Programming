#!/bin/bash
in_file=$1
#mpicxx basic.cc -O3 -o basic -lm
#mpirun -np 4 ./basic $in_file
g++ basic.cc -O3 -o basic -lm
./basic $in_file
