package pageRank;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;

import org.apache.hadoop.io.FloatWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.TaskCounter;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;
import java.io.IOException;
import java.text.DecimalFormat;
import java.text.NumberFormat;

import pageRank.Parse;
import pageRank.Cal;
import pageRank.Sort;

public class pageRank{
	// declare some parameters
    public static int NumReducer = 32;
	public static Double damping_ratio = 0.85;

    private static NumberFormat nf = new DecimalFormat("00");
    
    public static long PageNum;
    public static long DanglingNum;
	
	public static int main(String[] args) throws Exception{
        String newest_Path = null;
        String in_Path = args[0];
		String out_Path = args[1] + "/iter";
		
        /******************************* Parse XML **********************************/
        Parse parse_job = new Parse();
		boolean isCompleted = parse_job.Parse(in_Path, out_Path + "00");
		//boolean isCompleted = parse_job.Parse(in_Path, tmpIn_Path);
		PageNum = parse_job.getPageNum();
		DanglingNum = parse_job.getDanglingNum();
		System.out.println("Total Page num: " + PageNum + ", total dangling nodes: " + DanglingNum);
        if(!isCompleted){
            System.out.println("[FATAL] In 'Parse' - got some problems\n\n");        
			System.exit(-1);
		}
        newest_Path = out_Path + "00";
	
        /**************************** Calculate PageRank *****************************/
        double err = 0.0;
        // check the second argument, if it's -1, then run until converge, otherwise to 
		// maximum iteration
        int ITERMAX = Integer.parseInt(args[2]);
		int iter = 0;
		
		// if we want to run until convergence...
		while(ITERMAX == -1){
			// define path first
			in_Path = out_Path + nf.format(iter);
			newest_Path = out_Path + nf.format(iter + 1);
			// start calculation
			Cal cal_rank = new Cal();
            err = cal_rank.Cal(in_Path, newest_Path, PageNum, DanglingNum);
			System.out.println("Error at iteration " + (iter + 1) + ": " + err);
			if(err < 0.001){
				System.out.println("Converge at iteration " + (iter + 1));
				break;	
			}
			iter++;
			// delete file 
			if(iter >= 2){
				Process p = Runtime.getRuntime().exec("hdfs dfs -rm -r "+ out_Path + nf.format(iter - 2));
				p.waitFor();
				System.out.println("Deleting file: " + out_Path + nf.format(iter - 2));
			}
		}
		
		// otherwise, we want to run with defined ITERMAX
        for(iter = 0; iter < ITERMAX; iter++){
			// define path
            in_Path = out_Path + nf.format(iter);
            newest_Path = out_Path + nf.format(iter + 1);
			// start calculation
            Cal cal_rank = new Cal();
			err = cal_rank.Cal(in_Path, newest_Path, PageNum, DanglingNum);
            System.out.println("Error at iteration " + (iter + 1) + ": " + err);
            if (err < 0.001) {
                System.out.println("Converge at iteration: " + (iter + 1));
                break;
            }
			// delete file 
			if(iter >= 2){
				Process p = Runtime.getRuntime().exec("hdfs dfs -rm -r "+ out_Path + nf.format(iter - 2));
				p.waitFor();
				System.out.println("Deleting file: " + out_Path + nf.format(iter - 2));
			}
        }

        /************************************* Sort ************************************/
        // result path
        String result_Path = args[1] + "/result";
		
		Sort sort = new Sort();
		isCompleted = sort.Sort(newest_Path, result_Path);
		
        return 0;
    }
}
