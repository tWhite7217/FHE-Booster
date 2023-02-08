#!/bin/bash

result_file_suffix=""

if [[ $(($4)) > 1 ]]; then
    result_file_suffix="_$4_cores"
fi

for i in $(seq $2 $3)
do
    
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i
    
    echo "TAKE D:\LINGO64_19\FHE_Model\FHE_Model_$1.lng
ALTER ALL 'custom_graph1'random_graph${i}'
ALTER ALL '_0_levels'_$6_levels'
ALTER ALL 'C1..C1'C1..C$4'
ALTER ALL 'bootstrap_latency = 300;'bootstrap_latency = $5;'
    DIVERT D:\LINGO64_19\FHE_Model\results\random_graph$i\rg${i}_$1${result_file_suffix}_$6_levels.lgr" > model_customizer.ltf
    
    powershell.exe "RunLingo .\run_solver.ltf"
    
done
