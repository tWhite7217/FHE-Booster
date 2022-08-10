#!/bin/bash

make tgff_to_unprocessed_lingo
make input_file_processor
seed=$(date +%s | awk '{ print substr ($0, 4 ) }')
echo "seed $seed" > ddg_generator.tgffopt
cat ddg_generator_base.tgffopt >> ddg_generator.tgffopt
mkdir DDGs/$1
/mnt/d/tgff-3.6/tgff ddg_generator
./CPP_code/tgff_to_unprocessed_lingo ddg_generator.tgff > DDGs/$1/$1.txt
./generate_both_LDT_variants.sh $1
rm ddg_generator.tgff
rm ddg_generator.eps
mv ddg_generator.vcg DDGs/$1/$1.vcg