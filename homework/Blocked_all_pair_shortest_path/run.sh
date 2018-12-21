#!/bin/bash
version=$1
testfile=$2
output_path=$3
srun -n 2 -p pp --gres=gpu:1 ./${version} ${testfile} ${output_path} 
#CUDA_VISIBLE_DEVICES=0,1 ./${version} ${testfile} ${output_path} 

