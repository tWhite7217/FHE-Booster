#pragma once

#include "program.h"

class FileWriter
{
public:
    FileWriter(const std::reference_wrapper<const Program>);

    void write_segments_to_file(const std::string &) const;
    void write_segments_to_text_file(const std::string &) const;
    void write_ldt_info_to_file(const std::string &) const;
    void write_lgr_info_to_file(const std::string &, int) const;
    void write_bootstrapping_set_to_file(const std::string &) const;
    void write_sched_file(const std::string &) const;

private:
    struct StartTimeCmp
    {
        bool operator()(const OperationPtr &a, const OperationPtr &b) const
        {
            return a->start_time < b->start_time;
        }
    };

    using SchedDataStructure = std::map<int, std::set<OperationPtr, StartTimeCmp>>;

    std::reference_wrapper<const Program> program_ref;

    void write_segments_to_file(std::ofstream &) const;
    void write_segments_to_text_file(std::ofstream &) const;

    void write_ldt_info_to_file(std::ofstream &) const;
    void write_operation_list_to_ldt_string_stream(std::ostringstream &) const;
    void write_operation_types_to_ldt_string_stream(std::ostringstream &) const;
    void write_operation_dependencies_to_ldt_string_stream(std::ostringstream &) const;
    void write_bootstrapping_constraints_to_ldt_string_stream(std::ostringstream &) const;
    void write_data_separator_to_ldt_string_stream(std::ostringstream &) const;

    void write_lgr_info_to_file(std::ofstream &, int) const;

    void write_bootstrapping_set_to_file(std::ofstream &) const;
    void write_bootstrapping_set_to_file_complete_mode(std::ofstream &) const;
    void write_bootstrapping_set_to_file_selective_mode(std::ofstream &) const;

    void write_sched_file(std::ofstream &) const;
    std::pair<std::string, std::string> get_arguments(const OperationPtr &) const;
    std::string get_constant_arg(const OperationPtr &, size_t) const;
    std::string get_variable_arg(const OperationPtr &, size_t) const;
    SchedDataStructure get_sched_data_from_program() const;
};