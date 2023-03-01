#include "file_writer.h"

FileWriter::FileWriter(const std::reference_wrapper<const Program> program_ref)
    : program_ref{program_ref} {}

void FileWriter::write_segments_to_file(const std::string &output_filename) const
{
    std::ofstream output_file(output_filename);
    write_segments_to_file(output_file);
    output_file.close();
}

void FileWriter::write_segments_to_file(std::ofstream &file) const
{
    const auto &bootstrap_segments = program_ref.get().bootstrap_segments;
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

void FileWriter::write_segments_to_text_file(const std::string &output_filename) const
{
    std::ofstream output_file(output_filename);
    write_segments_to_text_file(output_file);
    output_file.close();
}

void FileWriter::write_segments_to_text_file(std::ofstream &file) const
{
    std::ostringstream out_string_stream;

    const auto &bootstrap_segments = program_ref.get().bootstrap_segments;
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

    std::vector<std::function<void(std::ofstream &)>> write_functions = {
        [this](std::ofstream &file)
        { write_operation_list_to_ldt_file(file); },
        [this](std::ofstream &file)
        { write_operation_types_to_ldt_file(file); },
        [this](std::ofstream &file)
        { write_operation_dependencies_to_ldt_file(file); },
        [this](std::ofstream &file)
        { write_bootstrapping_constraints_to_ldt_file(file); }};

    for (auto write_data_func : write_functions)
    {
        write_data_func(output_file);
        write_data_separator_to_ldt_file(output_file);
    }

    output_file.close();
}

void FileWriter::write_operation_list_to_ldt_file(std::ofstream &file) const
{
    for (auto operation : program_ref.get())
    {
        file << "OP" << operation->id << std::endl;
    }
}

void FileWriter::write_operation_types_to_ldt_file(std::ofstream &file) const
{
    for (auto operation : program_ref.get())
    {
        auto operation_type_num = operation->type;
        for (size_t i = 0; i < OperationType::num_types_except_bootstrap; i++)
        {
            if (i == operation_type_num)
            {
                file << "1 ";
            }
            else
            {
                file << "0 ";
            }
        }
        file << std::endl;
    }
}

void FileWriter::write_operation_dependencies_to_ldt_file(std::ofstream &file) const
{
    for (auto operation : program_ref.get())
    {
        for (const auto &parent : operation->parent_ptrs)
        {
            file << "OP" << parent->id << " OP" << operation->id << std::endl;
        }
    }
}

void FileWriter::write_bootstrapping_constraints_to_ldt_file(std::ofstream &file) const
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
        file << constraint_string << std::endl;
    }
}

void FileWriter::write_data_separator_to_ldt_file(std::ofstream &file) const
{
    file << "~" << std::endl;
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
    for (auto operation : program)
    {
        file << "START_TIME( OP" << operation->id << ") " << operation->start_time << std::endl;
    }

    for (auto operation : program)
    {
        if (operation->core_num > 0)
        {
            file << "B2C( OP" << operation->id << ", C" << operation->core_num << ") 1" << std::endl;
        }
    }

    for (auto operation : program)
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
    for (auto operation : program_ref.get())
    {
        if (operation->is_bootstrapped())
        {
            file << "BOOTSTRAPPED( OP" << operation->id << ") 1" << std::endl;
        }
    }
}

void FileWriter::write_bootstrapping_set_to_file_selective_mode(std::ofstream &file) const
{

    for (auto operation : program_ref.get())
    {
        for (auto child : operation->bootstrap_children)
        {
            file << "BOOTSTRAPPED( OP" << operation->id << ", OP" << child->id << ") 1" << std::endl;
        }
    }
}