package pageRank;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Partitioner;

public class ParsePartitioner extends Partitioner<Text,Text>{
    public int getPartition(Text key, Text value, int ReduceTasks_num) {
		if(key.toString().length()>0 && key.toString().charAt(0)==' ')
			return Integer.parseInt(key.toString().substring(1, key.toString().length()));
		else if(key.toString().length() > 0)
			return (int)(key.toString().charAt(0)) % ReduceTasks_num;
		else return 0;
	}
}
