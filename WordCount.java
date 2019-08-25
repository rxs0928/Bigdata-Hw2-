/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Modified by Shimin Chen to demonstrate functionality for Homework 2
// April-May 2015

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
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

  // This is the Mapper class
  // reference: http://hadoop.apache.org/docs/r2.6.0/api/org/apache/hadoop/mapreduce/Mapper.html
  //
  public static class TokenizerMapper 
       extends Mapper<Object, Text, Text, IntWritable>{
    
    private final static DoubleWritable time = new DoubleWritable();
    private Text word = new Text();
      
    public void map(Object key, Text value, Context context
                    ) throws IOException, InterruptedException {
      String[] itr = value.toString().split("\\s+"); // 对一行内容进行拆分，\\s+表示一到多个空格,回车,换行等空白符， 
      
      if (itr.Length()!=3){
    	  return;
      }// noisy row,abondon it!
      
      String sdkey=itr[0]+" "+itr[1];
      word.set(sdkey);
      time.set(Double.parseDouble(itr[2]));
     
      context.write(word, time);
     
    }
  }
  
 /* public static class RowMergeCombiner
       extends Reducer<Text,DoubleWritable,Text,IntWritable> {
    private IntWritable result = new IntWritable();

    public void reduce(Text key, Iterable<IntWritable> values,
                       Context context
                       ) throws IOException, InterruptedException {
      int sum = 0;
      for (IntWritable val : values) {
        sum += val.get();
      }
      result.set(sum);
      context.write(key, result);
    }
  }*/

  // This is the Reducer class
  // reference http://hadoop.apache.org/docs/r2.6.0/api/org/apache/hadoop/mapreduce/Reducer.html
  //
  // We want to control the output format to look at the following:
  //
  // count of word = count
  //
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
