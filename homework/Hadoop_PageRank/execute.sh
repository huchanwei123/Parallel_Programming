#!/bin/bash
name=$1
ITER=$2

INPUT_FILE=/user/ta/PageRank/input-$name
OUTPUT_FILE=./output/pagerank_$name.out
RESULT_FILE=./output/pagerank_$name.out/result
JAR=pageRank.jar

hdfs dfs -rm -r $OUTPUT_FILE
hadoop jar $JAR pageRank.pageRank $INPUT_FILE $RESULT_FILE $ITER

hdfs dfs -getmerge $RESULT_FILE/result pagerank_$name.out
