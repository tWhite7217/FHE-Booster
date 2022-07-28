%class-name = "LGRParser"

%%

"BOOTSTRAPPED( OP"[0-9]+")"[ \t]+"1" {
    int operation_id = get_first_operation_id(matched());
    bootstrapped_operation_ids.push_back(operation_id);
}

"BOOTSTRAPPED( OP"[0-9]+", OP"[0-9]+")"[ \t]+"1" {
    used_selective_model = true;
    int operation_id1 = get_first_operation_id(matched());
    int operation_id2 = get_second_operation_id(matched());
    bootstrapped_operation_ids.push_back(operation_id1);
    bootstrapped_operation_ids.push_back(operation_id2);
}

"BOOTSTRAP_FINISH_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {}

"FINISH_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int finish_time = get_time(matched());
    max_finish_time = std::max(max_finish_time, finish_time);
}

"BOOTSTRAP_START_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int b_start_time = get_time(matched());
    int operation_id = get_first_operation_id(matched());
    bootstrap_start_times[operation_id] = b_start_time;
}

"START_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int start_time = get_time(matched());
    int operation_id = get_first_operation_id(matched());
    start_times[operation_id] = start_time;
}

"B2C( OP"[0-9]+", C"[0-9]+")"[ \t]+"1" {
    used_bootstrap_limited_model = true;
    int operation_id = get_first_operation_id(matched());
    int core_num = get_core_num(matched());
    cores_used[operation_id] = core_num;
}

.|\n {}

%%