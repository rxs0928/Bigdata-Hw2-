/**
 * This file implements count & average of multiple value between the same source and destination.
 *
 * @author Ren Xueshuang
 *
 */
// Modified by Shimin Chen to demonstrate functionality for Homework 2
// April-May 2018

import java.io.IOException;
import java.util.StringTokenizer;
import java.util.regex.Pattern;


import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapred.TextInputFormat;
import org.apache.hadoop.mapred.TextOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;

public class Hw2Part1 {

  // 

  /** 
   * This is the Mapper class.
   * If the number of strings in value is not 3 or the third string is not a double,then abondon it!
   * 
  * @#see TokenizerMapper <br>
  */
  public static class TokenizerMapper 
       extends Mapper<Object, Text, Text, DoubleWritable>{
    
    private final static DoubleWritable time = new DoubleWritable();
    private Text word = new Text();
      
    public void map(Object key, Text value, Context context
                    ) throws IOException, InterruptedException {
      String[] itr = value.toString().split("\\s+");
      
      if (itr.length!=3 || isDouble(itr[2])!=true){
    	  return;
      }// noisy row,abondon it!
      
      String sdkey=itr[0]+" "+itr[1];
      word.set(sdkey);
      time.set(Double.parseDouble(itr[2]));
     
      context.write(word, time);
     
    }
  }
  
  /**  
   * This is the Reducer class.
   *  @#see CountAvgReducer <br>
  */
  public static class CountAvgReducer
       extends Reducer<Text,DoubleWritable,Text,Text> {

    private Text result_key= new Text();
    private Text result_value= new Text();
  
    public void reduce(Text key, Iterable<DoubleWritable> values, 
                       Context context
                       ) throws IOException, InterruptedException {
      int sum = 0;
      double totalTime=0.0;
      for (DoubleWritable val : values) {
    	  sum+=1;
    	  totalTime += val.get();
      }

      // generate result key
      result_key.set(key);
           
      // generate result value
      double avgTime= totalTime/(double)sum;
      String resValue=new String();
      resValue= Integer.toString(sum) + " " + String.format("%.3f", avgTime);
      result_value.set(resValue);
      
      context.write(result_key, result_value);
    }
  }

 /** 
  * @#see isDouble <br>
  * @param str a string 
  * @return true or false
  */
 private static boolean isDouble(String str) {  
    if (null == str || "".equals(str)) {  
        return false;  
    }  
    Pattern pattern = Pattern.compile("^[-\\+]?[.\\d]*$");  
    return pattern.matcher(str).matches();  
 } 

  /**  
   * @#see removeOutPath <br>
  */

  public static void removeOutPath(String path, Configuration conf) throws IOException
  {
        FileSystem fs = FileSystem.get(conf);
        Path fpath = new Path(path);
        if(fs.exists(fpath)) {
		fs.delete(fpath, true);
         }
    }


  /**
  * @param args include <input file> <output directory>
	
  */
  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
    if (otherArgs.length < 2) {
      System.err.println("Usage: Hw2Part1 <in> [<in>...] <out>");
      System.exit(2);
    }

    Job job = Job.getInstance(conf, "Hw2Part1");

    job.setJarByClass(Hw2Part1.class);

    job.setMapperClass(TokenizerMapper.class);
    job.setReducerClass(CountAvgReducer.class);

    job.setMapOutputKeyClass(Text.class);
    job.setMapOutputValueClass(DoubleWritable.class);

    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(Text.class);

    // add the input paths as given by command line
    for (int i = 0; i < otherArgs.length - 1; ++i) {
      FileInputFormat.addInputPath(job, new Path(otherArgs[i]));
    }

    // add the output path as given by the command line
    FileOutputFormat.setOutputPath(job,
      new Path(otherArgs[otherArgs.length - 1]));
	
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}

