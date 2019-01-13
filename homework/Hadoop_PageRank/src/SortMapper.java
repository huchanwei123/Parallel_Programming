/**************************************************************************************************
Description:
	Map input:
		key:   Page
		value: PR|linkA|linkB|linkC...
**************************************************************************************************/
package pageRank;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;
import java.util.StringTokenizer;
import java.io.IOException;

public class SortMapper extends Mapper<Text, Text, SortPair, NullWritable> {
    public void map(Text key, Text value, Context context) throws IOException, InterruptedException {
		SortPair sp;
        if(value.toString().contains("|")){
            String[] content = value.toString().split("\\|", 2);
          	sp = new SortPair(key, Double.parseDouble(content[0]));
        }else{
        	sp = new SortPair(key, Double.parseDouble(value.toString()));
		}
        context.write(sp, NullWritable.get());
    }
}
