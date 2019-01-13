package pageRank;

import java.io.IOException;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;
import java.util.HashSet;

public class ParseReducer extends Reducer<Text, Text, Text, Text>{
	// declare the hashset
	private static HashSet<String> hs = new HashSet<String>();
	private static long count = 0;

	public static enum counterNums{
		Dangling
    }

    public void reduce(Text key, Iterable<Text> value, Context context) throws IOException, InterruptedException {
		StringBuilder builder = new StringBuilder();
		for(Text val: value){
			if(key.toString().charAt(0) == ' '){
				hs.add(val.toString());
			}else{
				if(hs.contains(val.toString()))
					builder.append( "|" + val.toString());
			}
		}
		if(key.toString().charAt(0) != ' '){
			Text write_out = new Text(String.valueOf(((double)1)/(hs.size())) + builder.toString());
			
			context.write(key, write_out);
			// dangling 
			if(!builder.toString().contains("|")) 
				context.getCounter(counterNums.Dangling).increment(1);
				//count++;
		}
		//context.getCounter(counterNums.Dangling).setValue(count);
	}
}
