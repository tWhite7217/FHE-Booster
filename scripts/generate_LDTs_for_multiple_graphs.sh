#!/bin/bash

for i in $(seq $1 $2)
do
    ./scripts/generate_both_LDT_variants.sh random_graph$i $3
done