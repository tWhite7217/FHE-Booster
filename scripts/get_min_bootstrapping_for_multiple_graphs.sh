#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <num_levels>

first_graph_num=$1
last_graph_num=$2
num_levels=$3

result_file_suffix=""

for i in $(seq $first_graph_num $last_graph_num)
do
    
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i/${num_levels}_levels
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i/${num_levels}_levels/bootstrapping_sets
    
    echo "TAKE D:\LINGO64_19\FHE_Model\FHE_Model_min_bootstrapping.lng
ALTER ALL 'custom_graph1'random_graph${i}'
ALTER ALL '0_levels'${num_levels}_levels'
    DIVERT D:\LINGO64_19\FHE_Model\results\random_graph$i\\${num_levels}_levels\bootstrapping_sets\rg${i}_min_bootstrapping.lgr" > model_customizer.ltf
    
    powershell.exe "RunLingo .\run_solver.ltf"
    
done
