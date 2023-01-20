#!/bin/bash

make input_file_processor.out

mkdir DAGs/$1/$2_levels

./CPP_code/input_file_processor.out DAGs/$1/$1.txt DAGs/$1/$2_levels/$1.LDT False $2
./CPP_code/input_file_processor.out DAGs/$1/$1.txt DAGs/$1/$2_levels/$1_selective.LDT True $2