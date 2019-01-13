#!/bin/bash

# read in argument
DATA_SIZE=$1

# build the code first
rm pageRank.jar
rm -rf bin
make 

# run it using script (only 1 iteration for DEBUG)
rm pagerank_${DATA_SIZE}.out
./execute.sh ${DATA_SIZE} 1

# judge the result
hw5-judge ${DATA_SIZE} 1 pagerank_${DATA_SIZE}.out
