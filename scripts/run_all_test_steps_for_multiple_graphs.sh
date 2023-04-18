#!/bin/bash

first_graph_num=$1
last_graph_num=$2
bootstrap_levels=$3
initial_levels=$4
num_cores=$5
first_save_num=$6
last_save_num=$7
inputs=$8
levels="b${bootstrap_levels}_i${initial_levels}"

./scripts/generate_segments_for_multiple_graphs.sh $first_graph_num $last_graph_num $bootstrap_levels $initial_levels
./scripts/generate_LDTs_for_multiple_graphs.sh $first_graph_num $last_graph_num $levels
./scripts/get_min_bootstrapping_for_multiple_graphs.sh $first_graph_num $last_graph_num $levels
./scripts/generate_and_convert_all_bootstrap_sets_for_multiple_graphs.sh $first_graph_num $last_graph_num $levels
./scripts/list_schedule_all_heuristics_for_multiple_graphs.sh $first_graph_num $last_graph_num $num_cores $levels
./scripts/run_sched_all_heuristics_for_multiple_graphs.sh $first_graph_num $last_graph_num $num_cores $bootstrap_levels $initial_levels $first_save_num $last_save_num $inputs