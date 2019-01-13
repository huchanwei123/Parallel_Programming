package pageRank;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.io.Text;

public class SortPair implements WritableComparable{
	private Text title = new Text();
	private double pr = 0.0;

	public SortPair() {
		title = new Text();
		pr = 0.0;
	}

	public SortPair(Text title, double pr) {
		this.title = title;
		this.pr = pr;
	}

	@Override
	public void write(DataOutput out) throws IOException{
		title.write(out);
		out.writeDouble(this.pr);
	}

	@Override
	public void readFields(DataInput in) throws IOException{
		this.title.readFields(in);
		this.pr = in.readDouble();
	}

	public Text getTitle() {
		return title;
	}

	public double getPR() {
		return pr;
	}

	@Override
	public int compareTo(Object o) {

		double thisPR = this.getPR();
		double thatPR = ((SortPair)o).getPR();

		Text thisTitle = this.getTitle();
		Text thatTitle = ((SortPair)o).getTitle();

		if(thisPR < thatPR) 
			return 1;
		else if(thisPR > thatPR) 
			return -1;
		else 
			return thisTitle.toString().compareTo(thatTitle.toString());
	}
} 
