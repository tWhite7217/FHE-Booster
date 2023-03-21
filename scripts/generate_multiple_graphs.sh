#!/bin/bash

first_graph_num=$1
increment=$2
last_graph_num=$3
initial_num_operations=$4
graph_gen_options=$5

make random_graph_generator.out
make txt_to_vcg.out
make input_file_processor.out

for i in $(seq $first_graph_num $increment $last_graph_num)
do
    for j in $(seq 1 $increment)
    do
        
        graph_num=$((i-1+j))
        op_offset=$((((i-$first_graph_num)/$increment)*10))
        num_operations=$(($initial_num_operations+op_offset))
        
        echo $graph_num
        echo $op_offset
        echo $num_operations
        
        output_file_path=DAGs/random_graph$graph_num/random_graph$graph_num
        
        mkdir DAGs/random_graph$graph_num
        echo ./CPP_code/random_graph_generator.out $output_file_path $num_operations $graph_gen_options
        ./CPP_code/random_graph_generator.out $output_file_path $num_operations $graph_gen_options
        echo ./CPP_code/txt_to_vcg.out $output_file_path.txt $output_file_path.vcg
        ./CPP_code/txt_to_vcg.out $output_file_path.txt $output_file_path.vcg
        # ./scripts/generate_both_LDT_variants.sh random_graph$graph_num ${12}
        sleep 1
        
    done
done