#!/bin/bash

for i in $(seq 1 100)
do
    
    sed -i 1,3d ./DDGs/random_graph$i/random_graph$i.LDT
    sed -i 1,3d ./DDGs/random_graph$i/random_graph${i}_selective.LDT
    sed -i -z 's/~\n12\n//' ./DDGs/random_graph$i/random_graph$i.LDT
    sed -i -z 's/~\n12\n//' ./DDGs/random_graph$i/random_graph${i}_selective.LDT
    
done