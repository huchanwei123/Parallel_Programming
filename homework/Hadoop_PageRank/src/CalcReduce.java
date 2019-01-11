package pageRank;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.TaskCounter;
import org.apache.hadoop.mapreduce.Cluster;
import org.apache.hadoop.mapreduce.Reducer;
import java.io.IOException;

import pageRank.Cal;

public class CalcReduce extends Reducer<Text, Text, Text, Text> {

    public static final Double SCALING_FACTOR  = 1E18;
    public static final Double SCALING_COUNTER = 1E15;
    public static final Double SCALING = 1E14;

    public static final Double DAMPING_FACTOR  = 0.85;
    public static double ErrorSum;
    Double DanglingSum;
    Long ScaledDanglingSum;
    Long PageNum, DanglingNum;    
    Long scaledDelta;

    protected void setup(Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration();
        PageNum   = conf.getLong("PageNum", 0);
        DanglingNum = conf.getLong("DanglingNum", 0);
        
        Cluster cluster = new Cluster(conf);
        Job currentJob = cluster.getJob(context.getJobID());
        ScaledDanglingSum = currentJob.getCounters().findCounter(Cal.counterTypes.DANGLINGSUM).getValue(); 
        DanglingSum = ScaledDanglingSum / SCALING_COUNTER;
        ErrorSum = 0.0;
        System.out.println("[CalcReducer Setup] PageNum: " + PageNum);
        System.out.println("[CalcReducer Setup] DanglingNum: " + DanglingNum);
        System.out.println("[CalcReducer Setup] ScaledDanglingSum: " + ScaledDanglingSum);
        System.out.println("[CalcReducer Setup] DanglingSum: " + DanglingSum);
    }

    @Override
    public void reduce(Text page, Iterable<Text> values, Context context) throws IOException, InterruptedException {

        Double oldPageRank = 0.0;
        String oldPageRankString = "";

        Double newPageRank = 0.0;
        Double newPageRankSum = 0.0;

        String outLinksString  = "";
        String fromPageTitle   = "";
        Double fromPageRank    = 0.0;
        Double fromTotalLinks  = 0.0;

        for (Text value : values) {
            if ( value.toString().contains("###") ) {
                // <thisPage>  3.123|linkA|linkB|linkC
                String oldString = value.toString().substring(3);
                if (oldString.contains("|") ) {
                    String[] content = oldString.split("\\|", 2);
                    oldPageRankString = content[0];       // 3.123
                    outLinksString    = content[1];       // linkA|linkB|linkC
                    oldPageRank = Double.parseDouble(oldPageRankString);
                }
                else {
                    oldPageRank = Double.parseDouble(oldString);
                }
            }
            else {
                if ( value.toString().contains("|") ) {
                    // Calculate Page rank
                    //   <thisPage>      <fromPage> | <fromPageRank> | <fromNumOfLinks>

                    String[] parts  = value.toString().split("\\|");
                    fromPageTitle   = parts[0];
                    fromPageRank    = Double.parseDouble(parts[1]);
                    fromTotalLinks    = Double.parseDouble(parts[2]);
                    // fromTotalLinks  = Integer.getInteger(parts[2]);

                    System.out.println("fromPageTitle: " + fromPageTitle);
                    System.out.println("fromPageRank: " + fromPageRank);
                    System.out.println("fromTotalLinks: " + fromTotalLinks);
                    
                    System.out.println("before newPageRankSum: " + newPageRankSum);
                    newPageRankSum = newPageRankSum + ( fromPageRank / fromTotalLinks);
                    System.out.println("after newPageRankSum: " + newPageRankSum);
                }
                else {
                    System.out.println("NO page links to this page..");
                    newPageRankSum = 0.0;
                    fromTotalLinks = 0.0;
                }
            }
        }

        newPageRank = (Double) ( (1 - DAMPING_FACTOR)  / PageNum ) +  DAMPING_FACTOR * (newPageRankSum +  (Double) DanglingSum / PageNum);
        System.out.println("[calcReducer] ErrorSum: " + ErrorSum);

        System.out.println("[calcReducer] (1st item): " + (Double) ( 0.15 / PageNum ));
        System.out.println("[calcReducer] (newPageRankSum): " + newPageRankSum);
        System.out.println("[calcReducer] (DanglingSum): " + DanglingSum);
        System.out.println("[calcReducer] (newPageRankSum + DanglingSum): " + (newPageRankSum + DanglingSum));

        ErrorSum = ErrorSum + Math.abs(newPageRank - oldPageRank);
        System.out.println("[calcReducer] oldPageRank: " + oldPageRank);
        System.out.println("[calcReducer] newPageRank: " + newPageRank);
        System.out.println("[calcReducer] difference : " + Math.abs(newPageRank - oldPageRank));

        // Update Page rank
        if (outLinksString.isEmpty()){
            context.write(page, new Text(newPageRank.toString() ));
        }
        else {
            context.write(page, new Text(newPageRank.toString() + "|" + outLinksString ));
        }
    }

	protected void cleanup(Context context) throws IOException, InterruptedException {
        scaledDelta = (long) (ErrorSum *  SCALING);
        System.out.println("[calcReducer cleanup] ErrorSum: " + ErrorSum);
        System.out.println("[calcReducer cleanup] scaledDelta: " + scaledDelta);
        // System.out.println("[calcReducer cleanup] DanglingSum: " + DanglingSum);
        // scaledDelta = 100L;
        context.getCounter(Cal.counterTypes.DANGLINGSUM).setValue(0L);
        context.getCounter(Cal.counterTypes.ERROR).setValue(scaledDelta);
	}
}
