#!/bin/bash

my_nums1=($(seq 3 40))
my_nums2=(6 7 11 12 16 17 21 22 26 27 31 32 36 37 41 42 46 47 51 52 56 57 61 62 66 67 71 72 76 77 81 82 86 87 91 92 96 97)
shopt -s globstar

for i in $(seq 0 37)
do
    x=$((i+3))
    
    mkdir ./DDGs/random_graph$x
    
    regex="s/${my_nums2[i]}/${my_nums1[i]}/"
    # rename -n 's/${my_nums1[i]}/${my_nums2[i]}/' ./DDGs/random_graph$i/*
    # rename -n 's/${i}/{i}/' ./DDGs/random_graph$i/*
    # echo $regex
    echo "rename -n $regex ./DDGs/random_graph${my_nums1[i]}/*"
    rename -v $regex ./DDGs/random_graph${my_nums1[i]}/*
    
done