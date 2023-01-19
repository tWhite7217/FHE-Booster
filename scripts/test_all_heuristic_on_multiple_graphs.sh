#!/bin/bash

# heuristics=("num_paths_with_slack" "urgency" "urgency_and_num_paths" "num_paths_minus_slack" "num_paths_minus_urgency")
heuristics=("num_paths_with_slack" "urgent_num_paths" "num_paths_and_slack" "urgent_num_paths_and_slack")
heuristic_nums=("1 0 0" "1 0 100" "2 1 0" "2 1 100")

./scripts/list_schedule_multiple.sh limited $1 $2 $3 limited_min_bootstrapping file -1 $4
./scripts/convert_multiple_to_selective.sh $1 $2 min_bootstrapping_$4_levels $4
./scripts/list_schedule_multiple.sh converted $1 $2 $3 selective_min_bootstrapping_converted file -1 $4

j=0

core_suffix=""

if [[ "$3" != "1" ]]; then
    core_suffix="_$3_cores"
fi

for i in "${heuristics[@]}"
do
    echo $j
    
    ./scripts/list_schedule_multiple.sh NULL $1 $2 $3 limited_$i heuristic ${heuristic_nums[j]} $4
    ./scripts/convert_multiple_to_selective.sh $1 $2 $4_levels_list_limited_${i}$core_suffix $4 _$i
    ./scripts/list_schedule_multiple.sh ${i}_converted $1 $2 $3 selective_${i}_converted file ${heuristic_nums[j]} $4
    ((j=j+1))
done
