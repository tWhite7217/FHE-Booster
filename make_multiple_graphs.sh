#!/bin/bash

for i in $(seq $1 $2)
do
    
    ./make_graph.sh random_graph$i
    sleep 2
    
done