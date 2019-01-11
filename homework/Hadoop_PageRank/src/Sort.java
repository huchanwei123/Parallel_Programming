package pageRank;
import org.apache.hadoop.conf.Configuration;
//import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;

//import org.apache.hadoop.io.FloatWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.TaskCounter;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
//import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
//import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.NullWritable;
import java.io.IOException;
//import java.text.DecimalFormat;
//import java.text.NumberFormat;

public class Sort{
	public boolean Sort(String in_path, String out_path) throws Exception {
        // only one reducer is used during Sort phase
		int NumReducer = 1;

        Configuration conf = new Configuration();
        conf.setInt("NumReducer", NumReducer);
        Job job = Job.getInstance(conf, "Sort");
        job.setJarByClass(pageRank.class);
       
        job.setInputFormatClass(KeyValueTextInputFormat.class);
        job.setMapperClass(SortMapper.class);
        job.setReducerClass(SortReducer.class);
        job.setNumReduceTasks(NumReducer);

		// set the output class of Mapper and Reducer
		job.setMapOutputKeyClass(SortPair.class);
		job.setMapOutputValueClass(NullWritable.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(DoubleWritable.class);

        // set IO
        FileInputFormat.setInputPaths(job, new Path(in_path));
        FileOutputFormat.setOutputPath(job, new Path(out_path));

        return job.waitForCompletion(true);
    }
}

