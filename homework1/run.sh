#!/bin/bash
#SBATCH -N 4
#SBATCH -n 36
file=basic
N=100000
in_file="/home/ta/hw1/testcases/35.in"
out_file="output/35.out"

rm $file
mpicc ${file}.c -O3 -g -o $file -lm -lrt
srun ./$file $N $in_file $out_file
#mpirun -np 4 ./$file $N $in_file $out_file

# If you want to run in batch partition, please use:
#	sbatch -p batch run.sh
