#include "shared_utils.h"

void remove_chars_from_string(std::string &str, std::vector<char> chars_to_remove)
{
    for (unsigned int i = 0; i < chars_to_remove.size(); i++)
    {
        str.erase(remove(str.begin(), str.end(), chars_to_remove[i]), str.end());
    }
}

int extract_number_from_string(std::string str, size_t start_index, size_t end_index)
{
    auto num_digits = end_index - start_index;
    std::string num_as_string = str.substr(start_index, num_digits);
    int num = std::stoi(num_as_string);
    return num;
}

std::vector<std::string> split_string_by_character(std::string str, char separator)
{
    std::vector<std::string> str_as_list;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, separator))
    {
        str_as_list.push_back(item);
    }
    return str_as_list;
}

bool arg_exists(const std::string &options_string, const std::string &short_form, const std::string &long_form)
{
    bool short_form_exists = options_string.find(" " + short_form + " ") != std::string::npos;
    bool long_form_exists = options_string.find(" " + long_form + " ") != std::string::npos;
    return short_form_exists || long_form_exists;
}

std::string get_arg(const std::string &options_string, const std::string &short_form, const std::string &long_form, const std::string &help_info)
{
    auto short_pos = options_string.find(short_form);
    auto long_pos = options_string.find(long_form);
    size_t start_pos;
    char expected_char;
    if (short_pos != std::string::npos)
    {
        start_pos = short_pos + short_form.size() + 1;
        expected_char = ' ';
    }
    else if (long_pos != std::string::npos)
    {
        start_pos = long_pos + long_form.size() + 1;
        expected_char = '=';
    }
    else
    {
        return "";
    }

    if (options_string.at(start_pos - 1) != expected_char)
    {
        std::cout << "Options must follow the format shown." << std::endl;
        std::cout << help_info << std::endl;
        exit(-1);
    }

    auto end_pos = options_string.substr(start_pos, options_string.size() - start_pos).find(" ");
    return options_string.substr(start_pos, end_pos);
}

bool bool_arg_converter(const std::string &arg_val)
{
    if (arg_val == "y")
    {
        return true;
    }
    else if (arg_val == "n")
    {
        return false;
    }

    throw;
}

void print_size_mismatch_error(const size_t &expected_size, const size_t &actual_size, const std::string &short_form, const std::string &long_form)
{
    std::cout << "Command line argument " << short_form << "/" << long_form << " has " << actual_size << "elements, but was expected to have " << expected_size << " elements." << std::endl;
    exit(1);
}