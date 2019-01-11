package pageRank;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Partitioner;

public class ParsePartitioner extends Partitioner<Text,Text> {
    public int getPartition(Text key, Text value, int numReduceTasks) {
		if(key.toString().charAt(0)==' '){
			// 
			return Integer.parseInt(key.toString().substring(1));
		}
		else
			return (key.hashCode() & Integer.MAX_VALUE) % numReduceTasks;
	}
}
