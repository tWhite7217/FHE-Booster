#!/bin/bash

./CPP_code/input_file_processor DDGs/$1/$1.txt DDGs/$1/$1.LDT False
./CPP_code/input_file_processor DDGs/$1/$1.txt DDGs/$1/$1_some_children.LDT True