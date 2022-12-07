#!/bin/bash

./CPP_code/input_file_processor DDGs/$1/$1.txt DDGs/$1/$1_$2_levels.LDT False
./CPP_code/input_file_processor DDGs/$1/$1.txt DDGs/$1/$1_selective_$2_levels.LDT True