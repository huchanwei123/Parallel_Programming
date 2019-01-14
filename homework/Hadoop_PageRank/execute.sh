#!/bin/bash
D_SIZE=$1
ITER=$2

INPUT_FILE=/user/ta/PageRank/input-$D_SIZE
OUTPUT_FILE=./output/pagerank_$D_SIZE.out
RESULT_FILE=./output/pagerank_$D_SIZE.out/result
JAR=pageRank.jar

hdfs dfs -rm -r $OUTPUT_FILE
hadoop jar $JAR pageRank.pageRank $INPUT_FILE $RESULT_FILE $ITER

hdfs dfs -getmerge $RESULT_FILE/result pagerank_$D_SIZE.out
