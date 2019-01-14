package pageRank;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.TaskCounter;
import org.apache.hadoop.mapreduce.Cluster;
import org.apache.hadoop.mapreduce.Reducer;
import java.io.IOException;

import pageRank.pageRank;
import pageRank.Cal;

public class CalReducer extends Reducer<Text, Text, Text, Text> {
	public static final Double damping_ratio = pageRank.damping_ratio;
	public static double ErrorSum = 0.0;
    Double DanglingSum;
    Long ScaledDanglingSum;
    Long PageNum, DanglingNum;    
	Long scaled_err;

    protected void setup(Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration();
        PageNum   = conf.getLong("PageNum", 0);
        DanglingNum = conf.getLong("DanglingNum", 0);
        
        Cluster cluster = new Cluster(conf);
        Job currentJob = cluster.getJob(context.getJobID());
        ScaledDanglingSum = currentJob.getCounters().findCounter(Record.dan_sum).getValue();
		DanglingSum = ScaledDanglingSum / 1E15;
    }

    @Override
    public void reduce(Text page, Iterable<Text> values, Context context) throws IOException, InterruptedException {
        Double oldPR = 0.0;

        Double newPR = 0.0;
        Double newPRSum = 0.0;

        String outLinksString  = "";
        Double fromPageRank    = 0.0;
        Double fromTotalLinks  = 0.0;

        for(Text value : values){
            if(value.toString().contains("###")){
                String oldString = value.toString().substring(3);
                if(oldString.contains("|")){
                    String[] content = oldString.split("\\|", 2);
                    outLinksString = content[1];
					oldPR = Double.parseDouble(content[0]);
                }else{
                    oldPR = Double.parseDouble(oldString);
				}
            }else{
                if(value.toString().contains("|")){
                    String[] parts = value.toString().split("\\|");
                    fromPageRank = Double.parseDouble(parts[1]);
                    fromTotalLinks = Double.parseDouble(parts[2]);
                    newPRSum = newPRSum + (fromPageRank/fromTotalLinks);
                }else{
                    newPRSum = 0.0;
                    fromTotalLinks = 0.0;
                }
            }
        }

        newPR = (Double) ((1-damping_ratio)/PageNum) + damping_ratio * (newPRSum +  (Double)DanglingSum/PageNum);
        ErrorSum = ErrorSum + Math.abs(newPR - oldPR);
        // Update
        if(outLinksString.isEmpty())
            context.write(page, new Text(newPR.toString()));
        else
            context.write(page, new Text(newPR.toString() + "|" + outLinksString));
        
		scaled_err = (long) (ErrorSum * 1E14);
		context.getCounter(Record.dan_sum).setValue(0L);
		context.getCounter(Record.error).setValue(scaled_err);
	}
}
