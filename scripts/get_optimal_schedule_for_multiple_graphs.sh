#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <levels>

first_graph_num=$1
last_graph_num=$2
levels=$3
num_cores=$4

result_file_suffix=""

for i in $(seq $first_graph_num $last_graph_num)
do
    
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i/${levels}
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i/${levels}/optimal
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i/${levels}/optimal/${num_cores}_cores
    
echo "TAKE D:\LINGO64_19\FHE_Model\FHE_Model_cores_selective.lng
ALTER ALL 'custom_graph1'random_graph${i}'
ALTER ALL '0_levels'${levels}'
ALTER ALL 'C1..C1'C1..C${num_cores}'
    DIVERT D:\LINGO64_19\FHE_Model\results\random_graph$i\\${levels}\optimal\\${num_cores}_cores\rg${i}_selective.lgr" > model_customizer.ltf
    
    powershell.exe "RunLingo .\run_solver.ltf"
    
done
