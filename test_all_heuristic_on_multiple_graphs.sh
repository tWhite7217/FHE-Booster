#!/bin/bash

heuristics=("num_paths_with_slack" "urgency" "urgency_and_num_paths" "num_paths_minus_slack")

./list_schedule_multiple.sh limited $1 $2 $3 limited_min_bootstrapping file False -1
./convert_multiple_to_selective.sh $1 $2 list_limited_min_bootstrapping
./list_schedule_multiple.sh limited $1 $2 $3 selective_min_bootstrapping_converted file False -1

j=0

for i in "${heuristics[@]}"
do
    echo $j
    ./list_schedule_multiple.sh NULL $1 $2 $3 limited_$i heuristic False $j
    ./convert_multiple_to_selective.sh $1 $2 list_limited_$i _$i
    ./list_schedule_multiple.sh NULL $1 $2 $3 selective_${i}_converted file False $j
    ((j=j+1))
done


# ./list_schedule_multiple.sh limited $1 $2 $3 limited_min_bootstrapping file -1
# ./list_schedule_multiple.sh limited $1 $2 $3 limited_num_paths_with_slack heuristic 0
# ./list_schedule_multiple.sh limited $1 $2 $3 limited_urgency heuristic 1
# ./list_schedule_multiple.sh limited $1 $2 $3 limited_urgency_and_num_paths heuristic 2
# ./list_schedule_multiple.sh limited $1 $2 $3 limited_num_paths_minus_slack heuristic 3
# ./convert_multiple_to_selective.sh $1 $2 list_limited_min_bootstrapping _min_bootstrapping
# ./convert_multiple_to_selective.sh $1 $2 list_limited_num_paths_with_slack _num_paths_with_slack
# ./convert_multiple_to_selective.sh $1 $2 list_limited_urgency _urgency
# ./convert_multiple_to_selective.sh $1 $2 list_limited_urgency_and_num_paths _urgency_and_num_paths
# ./convert_multiple_to_selective.sh $1 $2 list_limited_num_paths_minus_slack _num_paths_minus_slack
# ./list_schedule_multiple.sh limited $1 $2 $3 selective_min_bootstrapping_converted file -1
# ./list_schedule_multiple.sh limited $1 $2 $3 selective_num_paths_with_slack_converted file 0
# ./list_schedule_multiple.sh limited $1 $2 $3 selective_urgency_converted file 1
# ./list_schedule_multiple.sh limited $1 $2 $3 selective_urgency_and_num_paths_converted file 2
# ./list_schedule_multiple.sh limited $1 $2 $3 selective_num_paths_minus_slack_converted file 3

