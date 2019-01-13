/**********************************************************************************
Description: 
	The input format will be: 
		PageTitle    PR | linkA | linkB | linkC ...
	Context write output:
		key: 	linkA or linkB or linkC ....
		value: 	current page | PR | Links_len
**********************************************************************************/
package pageRank;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.conf.Configuration;
import java.io.IOException;

import pageRank.pageRank;
import pageRank.Cal;

public class CalMapper extends Mapper<Text, Text, Text, Text> {

    private static Double Dangling_total_val = 0.0;
    Long PageNum = pageRank.PageNum;
	Long DanglingNum = pageRank.DanglingNum;
    
	public void map(Text key, Text value, Context context) throws IOException, InterruptedException{
        Configuration conf = context.getConfiguration(); 

        String curPage_title = key.toString();
        String curPage_val = value.toString();
        String curPage_rank = null;
        String curPage_link = null;

        // contains "|" means the page is NOT dangling page
        if(curPage_val.contains("|")){ 
			// split out the rank and links
            String[] content = curPage_val.split("\\|", 2);
            curPage_rank = content[0];     
            curPage_link = content[1]; 

            // multiple links in current page
            if(curPage_link.contains("|")){
				// split multiple out-going link
                String[] links = curPage_link.split("\\|");
                int Links_len = links.length;
                
                // start write the link info.
                for(String link : links){
                    StringBuilder curPage_output = new StringBuilder();
                    curPage_output.append(curPage_title + "|" + curPage_rank  + "|" + Integer.toString(Links_len));
                    context.write(new Text(link), new Text(curPage_output.toString()));
                }
            }else{
                // single link
                String link = curPage_link;
                StringBuilder curPage_output = new StringBuilder();
                curPage_output.append(curPage_title + "|" + curPage_rank  + "|" + Integer.toString(1));
                context.write(new Text(link), new Text(curPage_output.toString()));
            } 
        }else{
			// otherwise, it's dangling page (means that current page rank is curPage_val)
            Dangling_total_val += Double.parseDouble(curPage_val);
        }

        context.write(new Text(curPage_title), new Text("###" + value.toString()));
    	Long ScaledDtv = (long) (Dangling_total_val * 1E15);
		context.getCounter(Cal.Record.dan_sum).setValue(ScaledDtv);
	}
}
