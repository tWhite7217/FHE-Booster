#include "file_writer.h"

FileWriter::FileWriter(const std::reference_wrapper<const Program> program_ref)
    : program_ref{program_ref} {}

void FileWriter::write_segments_to_file(const std::vector<BootstrapSegment> &bootstrap_segments, const std::string &output_filename)
{
    std::ofstream output_file(output_filename);
    write_segments_to_file(bootstrap_segments, output_file);
    output_file.close();
}

void FileWriter::write_segments_to_file(const std::vector<BootstrapSegment> &bootstrap_segments, std::ofstream &file)
{
    size_t num_segments = bootstrap_segments.size();
    file.write((char *)(&num_segments), sizeof(size_t));
    std::vector<int> ids;

    size_t total_num_ids = 0;
    for (const auto &segment : bootstrap_segments)
    {
        size_t segment_size = segment.size();
        file.write((char *)(&segment_size), sizeof(size_t));
        total_num_ids += segment_size;
        for (const auto &operation : segment)
        {
            ids.push_back(operation->id);
        }
    }

    file.write((char *)(&ids[0]), sizeof(int) * total_num_ids);
}

void FileWriter::write_segments_to_text_file(const std::vector<BootstrapSegment> &bootstrap_segments, const std::string &output_filename)
{
    std::ofstream output_file(output_filename);
    write_segments_to_text_file(bootstrap_segments, output_file);
    output_file.close();
}

void FileWriter::write_segments_to_text_file(const std::vector<BootstrapSegment> &bootstrap_segments, std::ofstream &file)
{
    std::ostringstream out_string_stream;

    for (const auto &segment : bootstrap_segments)
    {
        for (const auto &operation : segment)
        {
            out_string_stream << operation->id << ",";
        }
        out_string_stream << "\n";
    }

    auto out_string = out_string_stream.str();

    file.write(out_string.c_str(), out_string.size());
}

void FileWriter::write_ldt_info_to_file(const std::string &filename) const
{
    std::ofstream output_file(filename);

    write_ldt_info_to_file(output_file);

    output_file.close();
}

void FileWriter::write_ldt_info_to_file(std::ofstream &file) const
{
    std::ostringstream string_stream;
    std::vector<std::function<void(std::ostringstream &)>> write_functions = {
        [this](std::ostringstream &stream)
        { write_operation_list_to_ldt_string_stream(stream); },
        [this](std::ostringstream &stream)
        { write_operation_types_to_ldt_string_stream(stream); },
        [this](std::ostringstream &stream)
        { write_operation_dependencies_to_ldt_string_stream(stream); },
        [this](std::ostringstream &stream)
        { write_bootstrapping_constraints_to_ldt_string_stream(stream); }};

    for (auto write_data_func : write_functions)
    {
        write_data_func(string_stream);
        write_data_separator_to_ldt_string_stream(string_stream);
    }

    auto ldt_string = string_stream.str();
    file.write(ldt_string.c_str(), ldt_string.size());
}

void FileWriter::write_operation_list_to_ldt_string_stream(std::ostringstream &stream) const
{
    for (const auto operation : program_ref.get())
    {
        stream << "OP" << operation->id << std::endl;
    }
}

void FileWriter::write_operation_types_to_ldt_string_stream(std::ostringstream &stream) const
{
    for (const auto operation : program_ref.get())
    {
        auto operation_type_num = operation->type;
        for (size_t i = 0; i < OperationType::num_types_except_bootstrap; i++)
        {
            if (i == operation_type_num)
            {
                stream << "1 ";
            }
            else
            {
                stream << "0 ";
            }
        }
        stream << std::endl;
    }
}

void FileWriter::write_operation_dependencies_to_ldt_string_stream(std::ostringstream &stream) const
{
    for (const auto operation : program_ref.get())
    {
        for (const auto &parent : operation->parent_ptrs)
        {
            stream << "OP" << parent->id << " OP" << operation->id << std::endl;
        }
    }
}

void FileWriter::write_bootstrapping_constraints_to_ldt_string_stream(std::ostringstream &stream) const
{
    const auto &program = program_ref.get();
    for (const auto &segment : program.bootstrap_segments)
    {
        std::string constraint_string;
        if (program.mode == BootstrapMode::SELECTIVE)
        {
            for (size_t i = 0; i < segment.size() - 1; i++)
            {
                if ((i > 0) && (i % 10 == 0))
                {
                    constraint_string += "\n";
                }
                constraint_string += "BOOTSTRAPPED(" + std::to_string(segment.operation_at(i)->id) + ", " + std::to_string(segment.operation_at(i + 1)->id) + ") + ";
            }
        }
        else
        {
            for (size_t i = 0; i < segment.size(); i++)
            {
                if ((i > 0) && (i % 20 == 0))
                {
                    constraint_string += "\n";
                }
                constraint_string += "BOOTSTRAPPED(" + std::to_string(segment.operation_at(i)->id) + ") + ";
            }
        }
        constraint_string.pop_back();
        constraint_string.pop_back();
        constraint_string += ">= 1;";
        stream << constraint_string << std::endl;
    }
}

void FileWriter::write_data_separator_to_ldt_string_stream(std::ostringstream &stream) const
{
    stream << "~" << std::endl;
}

void FileWriter::write_lgr_info_to_file(const std::string &filename, int total_latency) const
{
    std::ofstream output_file(filename);
    write_lgr_info_to_file(output_file, total_latency);
    output_file.close();
}

void FileWriter::write_lgr_info_to_file(std::ofstream &file, int total_latency) const
{
    file << "Objective value: " << total_latency << ".0" << std::endl;

    write_bootstrapping_set_to_file(file);

    const auto &program = program_ref.get();
    for (const auto operation : program)
    {
        file << "START_TIME( OP" << operation->id << ") " << operation->start_time << std::endl;
    }

    for (const auto operation : program)
    {
        if (operation->core_num > 0)
        {
            file << "O2C( OP" << operation->id << ", C" << operation->core_num << ") 1" << std::endl;
        }
    }

    for (const auto operation : program)
    {
        if (operation->bootstrap_start_time > 0)
        {
            file << "BOOTSTRAP_START_TIME( OP" << operation->id << ") " << operation->bootstrap_start_time << std::endl;
        }
    }

    file << "FINISH_TIME( OP0) " << total_latency << std::endl;
}

void FileWriter::write_bootstrapping_set_to_file(const std::string &filename) const
{
    std::ofstream output_file(filename);
    write_bootstrapping_set_to_file(output_file);
    output_file.close();
}

void FileWriter::write_bootstrapping_set_to_file(std::ofstream &file) const
{
    if (program_ref.get().mode == BootstrapMode::COMPLETE)
    {
        write_bootstrapping_set_to_file_complete_mode(file);
    }
    else
    {
        write_bootstrapping_set_to_file_selective_mode(file);
    }
}

void FileWriter::write_bootstrapping_set_to_file_complete_mode(std::ofstream &file) const
{
    for (const auto operation : program_ref.get())
    {
        if (operation->is_bootstrapped())
        {
            file << "BOOTSTRAPPED( OP" << operation->id << ") 1" << std::endl;
        }
    }
}

void FileWriter::write_bootstrapping_set_to_file_selective_mode(std::ofstream &file) const
{

    for (const auto operation : program_ref.get())
    {
        for (auto child : operation->bootstrap_children)
        {
            file << "BOOTSTRAPPED( OP" << operation->id << ", OP" << child->id << ") 1" << std::endl;
        }
    }
}

void FileWriter::write_sched_file(const std::string &output_filename) const
{
    std::ofstream output_file(output_filename);
    write_sched_file(output_file);
    output_file.close();
}

void FileWriter::write_sched_file(std::ofstream &file) const
{
    auto sched_data = get_sched_data_from_program();

    for (const auto &[core_num, operations_on_core] : sched_data)
    {
        for (const auto &operation : operations_on_core)
        {
            std::string result_var = " c" + std::to_string(operation->id);

            auto [arg1, arg2] = get_arguments(operation);

            std::string thread = " t" + std::to_string(core_num);

            file << operation->type.to_string() << result_var << arg1 << arg2 << thread << std::endl;
            if (operation->is_bootstrapped())
            {
                file << "BOOT c0" << operation->id << " c" << operation->id << " t" << core_num << std::endl;
            }
        }
    }
}

std::pair<std::string, std::string> FileWriter::get_arguments(const OperationPtr &operation) const
{
    std::string arg1;
    std::string arg2;

    auto num_var_parents = operation->parent_ptrs.size();
    auto num_const_parents = operation->constant_parent_ids.size();

    if (num_var_parents == 2)
    {
        arg1 = get_variable_arg(operation, 0);
        arg2 = get_variable_arg(operation, 1);
    }
    else if (num_const_parents == 2)
    {
        arg1 = get_constant_arg(operation, 0);
        arg2 = get_constant_arg(operation, 1);
    }
    else if (num_var_parents == 1 && num_const_parents == 1)
    {
        arg1 = get_variable_arg(operation, 0);
        arg2 = get_constant_arg(operation, 0);
    }
    else if (num_var_parents == 1)
    {
        arg1 = arg2 = get_variable_arg(operation, 0);
    }
    else if (num_const_parents == 1)
    {
        arg1 = arg2 = get_constant_arg(operation, 0);
    }
    else
    {
        throw std::runtime_error("An operation must have at least one input.");
    }

    return {arg1, arg2};
}

std::string FileWriter::get_constant_arg(const OperationPtr &operation, size_t parent_num) const
{
    return " k" + std::to_string(operation->constant_parent_ids[parent_num]);
}

std::string FileWriter::get_variable_arg(const OperationPtr &operation, size_t parent_num) const
{
    std::string arg = " c";
    auto parent = operation->parent_ptrs[parent_num];
    if (operation->receives_bootstrapped_result_from(parent))
    {
        arg += "0";
    }
    return arg + std::to_string(parent->id);
}

FileWriter::SchedDataStructure FileWriter::get_sched_data_from_program() const
{
    SchedDataStructure sched_data;
    for (const auto &operation : program_ref.get())
    {
        auto core = operation->core_num;
        // if (!sched_data.contains(core))
        // {
        //     sched_data.emplace(core);
        // }
        sched_data[core].insert(operation);
    }

    return sched_data;
}
