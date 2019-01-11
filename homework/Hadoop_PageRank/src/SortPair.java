package pageRank;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.io.Text;

public class SortPair implements WritableComparable{
	private Text word;
	private double pagerank;

	public SortPair() {
		word = new Text();
		pagerank = 0.0;
	}

	public SortPair(Text word, double pagerank) {
		this.word = word;
		this.pagerank = pagerank;
	}

	@Override
	public void write(DataOutput out) throws IOException {
		word.write(out);
		out.writeDouble(this.pagerank);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		this.word.readFields(in);
		this.pagerank = in.readDouble();
	}

	public Text getWord() {
		return word;
	}

	public double getPageRank() {
		return pagerank;
	}

	@Override
	public int compareTo(Object o) {

		double thisPageRank = this.getPageRank();
		double thatPageRank = ((SortPair)o).getPageRank();

		Text thisWord = this.getWord();
		Text thatWord = ((SortPair)o).getWord();

		// Compare between two objects
		// First order by pagerank, and then sort them lexicographically in ascending order
		if( thisPageRank < thatPageRank ) return 1;
		else if( thisPageRank > thatPageRank ) return -1;
		else return thisWord.toString().compareTo(thatWord.toString());
	}
} 
