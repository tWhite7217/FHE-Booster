#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <levels>

first_graph_num=$1
last_graph_num=$2
levels=$3

result_file_suffix=""

for i in $(seq $first_graph_num $last_graph_num)
do
    
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i/${levels}
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i/${levels}/min_bootstrapping
    
    echo "TAKE D:\LINGO64_19\FHE_Model\FHE_Model_min_bootstrapping.lng
ALTER ALL 'custom_graph1'random_graph${i}'
ALTER ALL '0_levels'${levels}'
    DIVERT D:\LINGO64_19\FHE_Model\results\random_graph$i\\${levels}\min_bootstrapping\complete_bootstrap_set.lgr" > model_customizer.ltf
    
    powershell.exe "RunLingo .\run_solver.ltf"
    
done
