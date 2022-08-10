#!/bin/bash

for i in $(seq $1 $2)
do
    
    num_lines=$(cat ./DDGs/random_graph$i/random_graph$i.txt | wc -l)
    num_lines=((num_lines-3))
    if (( num_lines > $3)); then
        echo "Graph $i has $num_lines lines"
    fi
    
done