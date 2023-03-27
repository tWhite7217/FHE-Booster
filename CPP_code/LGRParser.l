%class-name = "LGRParser"

%%

"BOOTSTRAPPED( OP"[0-9]+")"[ \t]+"1" {
    program_ref.get().set_boot_mode(BootstrapMode::COMPLETE);
    auto operation = get_first_operation_ptr(matched());
    operation->bootstrap_children = operation->child_ptrs;
}

"BOOTSTRAPPED( OP"[0-9]+", OP"[0-9]+")"[ \t]+"1" {
    program_ref.get().set_boot_mode(BootstrapMode::SELECTIVE);
    auto operation1 = get_first_operation_ptr(matched());
    auto operation2 = get_second_operation_ptr(matched());
    operation1->bootstrap_children.insert(operation2);
}

"BOOTSTRAP_FINISH_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {}

"FINISH_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int finish_time = get_time(matched());
    max_finish_time = std::max(max_finish_time, finish_time);
}

"BOOTSTRAP_START_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int b_start_time = get_time(matched());
    auto operation = get_first_operation_ptr(matched());
    operation->bootstrap_start_time = b_start_time;
}

"START_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int start_time = get_time(matched());
    auto operation = get_first_operation_ptr(matched());
    operation->start_time = start_time;
}

"B2C( OP"[0-9]+", C"[0-9]+")"[ \t]+"1" {
    used_bootstrap_limited_model = true;
    auto operation = get_first_operation_ptr(matched());
    int core_num = get_core_num(matched());
    operation->core_num = core_num;
}

"O2C( OP"[0-9]+", C"[0-9]+")"[ \t]+"1" {
    used_bootstrap_limited_model = true;
    auto operation = get_first_operation_ptr(matched());
    int core_num = get_core_num(matched());
    operation->core_num = core_num;
}

.|\n {}

%%