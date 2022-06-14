#!/bin/bash

seed=$(date +%s | awk '{ print substr ($0, 4 ) }')
echo "seed $seed" > ddg_generator.tgffopt
cat ddg_generator_base.tgffopt >> ddg_generator.tgffopt
/mnt/d/tgff-3.6/tgff ddg_generator
./tgff_to_unprocessed_lingo ddg_generator.tgff > $1.txt
python3 input_file_processor.py $1
rm ddg_generator.tgff
mv ddg_generator.vcg $1.vcg