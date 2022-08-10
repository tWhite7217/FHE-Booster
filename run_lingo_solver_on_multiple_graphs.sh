#!/bin/bash

result_file_suffix=""

if [[ $(($4)) > 1 && ("$1" == "limited" || "$1" == "selective") ]]; then
    result_file_suffix="_$4_cores"
fi

for i in $(seq $2 $3)
do
    
    mkdir /mnt/d/LINGO64_19/FHE_Model/results/random_graph$i
    
    echo "TAKE D:\LINGO64_19\FHE_Model\FHE_Model_$1.lng
ALTER ALL 'custom_graph1'random_graph$i'
ALTER ALL 'C1..C1'C1..C$4'
    DIVERT D:\LINGO64_19\FHE_Model\results\random_graph$i\rg${i}_$1$result_file_suffix.lgr" > model_customizer.ltf
    
    powershell.exe "RunLingo .\run_solver.ltf"
    
done
