#!/bin/bash

for i in $(seq $1 $2)
do
    
    num_lines=$(cat ./DDGs/random_graph$i/random_graph$i.txt | wc -l)
    ((num_lines=num_lines-3))
    echo $num_lines
    
done