package pageRank;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.LongWritable;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;
import java.util.StringTokenizer;
import java.io.IOException;

public class SortMapper extends Mapper<Text, Text, SortPair, NullWritable> {

    public void map(Text key, Text value, Context context) throws IOException, InterruptedException {
        // <key>    <value>
        //  Page     3.123|linkA|linkB|linkC

        String pageRank = "";
        if ( value.toString().contains("|") ) {
            // page with links
            String[] content = value.toString().split("\\|", 2);
            pageRank = content[0];       // 3.123
        } 
        else {
            // page without links
            pageRank = value.toString();
        }
        SortPair sp = new SortPair(key, Double.parseDouble(pageRank));
        context.write(sp, NullWritable.get());
    }
}
