#!/bin/bash

make input_file_processor.out

./CPP_code/input_file_processor.out DAGs/$1/$1.txt DAGs/$1/$1_$2_levels.LDT False $2
./CPP_code/input_file_processor.out DAGs/$1/$1.txt DAGs/$1/$1_selective_$2_levels.LDT True $2