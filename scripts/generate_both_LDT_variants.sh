#!/bin/bash

graph_name=$1
levels=$2

dag_dir="DAGs/$graph_name"
dag_file="$dag_dir/$graph_name.txt"
seg_dir="$dag_dir/$levels"
standard_segments_file="$seg_dir/bootstrap_segments_standard.dat"
selective_segments_file="$seg_dir/bootstrap_segments_selective.dat"
complete_output_file="$seg_dir/complete.LDT"
selective_output_file="$seg_dir/selective.LDT"

make ldt_generator.out

./CPP_code/ldt_generator.out $dag_file $standard_segments_file $complete_output_file COMPLETE
./CPP_code/ldt_generator.out $dag_file $selective_segments_file $selective_output_file SELECTIVE