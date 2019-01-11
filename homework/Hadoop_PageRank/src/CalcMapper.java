package pageRank;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.conf.Configuration;
import java.io.IOException;

import pageRank.pageRank;
import pageRank.Cal;

public class CalcMapper extends Mapper<Text, Text, Text, Text> {

    private static Double DanglingSum = 0.0;
    Long PageNum = pageRank.PageNum;
	Long DanglingNum = pageRank.DanglingNum;
    int n_reducer = pageRank.NumReducer;
    
	public void map(Text key, Text value, Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration(); 

        String thisPageTitle = key.toString();
        String thisPageValue = value.toString();
        String thisPageRank = null;
        String thisPageLink = null;

        // input 
        // <thisPageTitle>    3.254466755622091E-5|Consolation of Philosophy

        // NOT Dangling Page
        if (thisPageValue.contains("|") ) { 
            // Split content like <3.123|linkA|linkB|linkC>
            String[] content = thisPageValue.split("\\|", 2);
            thisPageRank = content[0];       // 3.123
            thisPageLink = content[1];       // linkA|linkB|linkC

            System.out.println("thisPageLink : " + thisPageLink);

            // more than 1 links
            if (thisPageLink.contains("|")) {
                
                String[] links = thisPageLink.split("\\|");
                int totalLinks = links.length;
                System.out.println("totalLinks : " + totalLinks);
                
                // Write out to calculate PR
                for (String link : links) {
                    System.out.println("this link : " + link);
                    StringBuilder thisPageWriteOut = new StringBuilder();
    
                    // <---KEY--->   <-----------------VALUE----------------> 
                    //   <linkA>      <thisPage> | <pageRank> | <numOfLinks>
                    //   <linkB>      <thisPage> | <pageRank> | <numOfLinks>
                    thisPageWriteOut.append( thisPageTitle + "|" + thisPageRank  + "|" + Integer.toString(totalLinks) );
                    
                    // Write out to calculate PR
                    context.write(new Text(link), new Text(thisPageWriteOut.toString() ));
                }
            }
            else {
                // only 1 link
                String link = thisPageLink;
                int totalLinks = 1;
                StringBuilder thisPageWriteOut = new StringBuilder();
                thisPageWriteOut.append( thisPageTitle + "|" + thisPageRank  + "|" + Integer.toString(totalLinks) );
                
                // Write out to calculate PR
                context.write(new Text(link), new Text(thisPageWriteOut.toString() ));
            } 
        }
        else { // Dangling Page
            thisPageRank = thisPageValue;
            DanglingSum += Double.parseDouble(thisPageRank);
            // System.out.println("dangling, PR: " + thisPageRank + " ,DanglingSum: " + DanglingSum );
        }

        // Write original input to context for calculating error
        context.write(new Text(thisPageTitle), new Text("###" + value.toString()) );
		
		Long ScaledDanglingSum = (long) (DanglingSum * 1E15);
		context.getCounter(Cal.counterTypes.DANGLINGSUM).setValue(ScaledDanglingSum);
    }
/*
	protected void cleanup(Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration();
        Long ScaledDanglingSum = (long) (DanglingSum * 1E15);

        // conf.setLong("ScaledDanglingSum", ScaledDanglingSum);
        System.out.println("[CalcMapper Cleanup] DanglingSum: " + DanglingSum );
        System.out.println("[CalcMapper Cleanup] ScaledDanglingSum: " + ScaledDanglingSum);
		context.getCounter(Cal.counterTypes.DANGLINGSUM).setValue(ScaledDanglingSum);
	}
*/
}
