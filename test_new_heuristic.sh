#!/bin/bash

./list_schedule_multiple.sh limited 1 100 1 limited_$1 heuristic
./convert_multiple_to_selective.sh 1 100 list_limited_$1 _$1
./list_schedule_multiple.sh $1_converted 1 100 1 selective_$1_converted

echo limited_results

./print_results_for_multiple_graphs.sh list_limited_$1 1 100

echo selective_results

./print_results_for_multiple_graphs.sh list_selective_$1_converted 1 100