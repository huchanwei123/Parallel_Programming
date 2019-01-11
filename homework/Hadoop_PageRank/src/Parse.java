package pageRank;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.TaskCounter;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;
import org.apache.hadoop.io.DoubleWritable;
import java.io.IOException;

// import my parse mapper and reducer
import pageRank.pageRank;
import pageRank.ParseMapper;
import pageRank.ParseReducer;

public class Parse{
	// total # of pages and # of dangling node
	private long PageNum;
	private long DanglingNum;
	
	public boolean Parse(String in_path, String out_path) throws Exception{
		Configuration conf = new Configuration();
		
		Job job = Job.getInstance(conf, "Parse");
		job.setJarByClass(pageRank.class);

		// set input format
		job.setInputFormatClass(TextInputFormat.class);
		
		// set each step 
		job.setMapperClass(ParseMapper.class);
		job.setPartitionerClass(ParsePartitioner.class);
		job.setReducerClass(ParseReducer.class);
		
		// set reducer number
		job.setNumReduceTasks(pageRank.NumReducer);

		// set output 
		job.setMapOutputKeyClass(Text.class);
        job.setMapOutputValueClass(Text.class);
        job.setOutputFormatClass(TextOutputFormat.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(Text.class);
        
		// add IO path
		FileInputFormat.addInputPath(job, new Path(in_path));
		FileOutputFormat.setOutputPath(job, new Path(out_path));

		// wait for job done
		if (!job.waitForCompletion(true)) throw new Exception("[Failed] In Parse :(");		
		
		// parse number
        PageNum = job.getCounters().findCounter(TaskCounter.MAP_INPUT_RECORDS).getValue();

        DanglingNum = job.getCounters().findCounter(ParseReducer.counterNums.Dangling).getValue();
		
		return true;
	}

	public long getPageNum(){
		return PageNum;
	}
	
	public long getDanglingNum(){
		return DanglingNum;
	}
}

