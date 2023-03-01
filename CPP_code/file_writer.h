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

private:
    std::reference_wrapper<const Program> program_ref;

    void write_segments_to_file(std::ofstream &) const;
    void write_segments_to_text_file(std::ofstream &) const;

    void write_operation_list_to_ldt_file(std::ofstream &) const;
    void write_operation_types_to_ldt_file(std::ofstream &) const;
    void write_operation_dependencies_to_ldt_file(std::ofstream &) const;
    void write_bootstrapping_constraints_to_ldt_file(std::ofstream &) const;
    void write_data_separator_to_ldt_file(std::ofstream &) const;

    void write_lgr_info_to_file(std::ofstream &, int) const;

    void write_bootstrapping_set_to_file(std::ofstream &) const;
    void write_bootstrapping_set_to_file_complete_mode(std::ofstream &) const;
    void write_bootstrapping_set_to_file_selective_mode(std::ofstream &) const;
};