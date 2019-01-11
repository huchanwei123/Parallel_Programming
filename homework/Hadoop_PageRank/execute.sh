#!/bin/bash
name=$1
ITER=$2

INPUT_FILE=/user/ta/PageRank/input-$name
OUTPUT_FILE=./output/pagerank_$name.out
RESULT_FILE=./output/pagerank_$name.out/result
JAR=pageRank.jar

hdfs dfs -rm -r $OUTPUT_FILE
hdfs dfs -rm -r $RESULT_FILE
hadoop jar $JAR pageRank.pageRank $INPUT_FILE $RESULT_FILE $ITER

echo "Merge files: pagerank_$name.out"
hdfs dfs -getmerge $RESULT_FILE/result pagerank_$name.out

# judge the result
hw5-judge $name $ITER pagerank_$name.out

