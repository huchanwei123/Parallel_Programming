package pageRank;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;

import org.apache.hadoop.io.FloatWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.TaskCounter;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;
import java.io.IOException;

import pageRank.pageRank;
import pageRank.CalMapper;
import pageRank.CalReducer;

public class Cal{
	public double Cal(String in_path, String out_path, long PageNum, long DanglingNum) throws Exception {
        
		Configuration conf = new Configuration();
        // set PageNum and DanglingNum to long type
		conf.setLong("PageNum", PageNum);
        conf.setLong("DanglingNum", DanglingNum);
		
		// create new job
        Job job = Job.getInstance(conf, "Cal");
        job.setJarByClass(pageRank.class);
        
		// set Mapper and Reducer class
        job.setMapperClass(CalMapper.class);
        job.setReducerClass(CalReducer.class);
        
        // set the output class of Mapper
        job.setMapOutputKeyClass(Text.class);
        job.setMapOutputValueClass(Text.class);

        job.setInputFormatClass(KeyValueTextInputFormat.class);
        job.setOutputFormatClass(TextOutputFormat.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(Text.class);
	
		// set reducer number
        job.setNumReduceTasks(pageRank.NumReducer);
        
		// set IO
		FileInputFormat.setInputPaths(job, new Path(in_path));
        FileOutputFormat.setOutputPath(job, new Path(out_path));

        if(!job.waitForCompletion(true)) throw new Exception("Calculation failed");

		long err_get = job.getCounters().findCounter(Record.error).getValue();
        double err = ((double) err_get/1E14);

	    return err;
    }
}
