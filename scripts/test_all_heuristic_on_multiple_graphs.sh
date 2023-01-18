#!/bin/bash

heuristics=("num_paths_with_slack" "urgency" "urgency_and_num_paths" "num_paths_minus_slack" "num_paths_minus_urgency")

./scripts/list_schedule_multiple.sh limited $1 $2 $3 limited_min_bootstrapping file -1 $4
./scripts/convert_multiple_to_selective.sh $1 $2 min_bootstrapping_$4_levels _$4_levels
./scripts/list_schedule_multiple.sh converted $1 $2 $3 selective_min_bootstrapping_converted file -1 $4

j=0

core_suffix=""

if [[ "$3" != "1" ]]; then
    core_suffix="_$3_cores"
fi

for i in "${heuristics[@]}"
do
    echo $j
    
    ./scripts/list_schedule_multiple.sh NULL $1 $2 $3 limited_$i heuristic $j $4
    ./scripts/convert_multiple_to_selective.sh $1 $2 $4_levels_list_limited_${i}$core_suffix _$4_levels_$i
    ./scripts/list_schedule_multiple.sh ${i}_converted $1 $2 $3 selective_${i}_converted file $j $4
    ((j=j+1))
done
