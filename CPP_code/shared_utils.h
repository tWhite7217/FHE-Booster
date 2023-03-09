#ifndef shared_utils_INCLUDED_
#define shared_utils_INCLUDED_

#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <fstream>
#include <functional>
#include <sstream>
#include <cmath>
#include <random>
#include <chrono>

enum class BootstrapMode
{
    COMPLETE,
    SELECTIVE
};

namespace utl
{
    void ltrim(std::string &);
    void rtrim(std::string &);
    void trim(std::string &);
    std::string get_trimmed_line_from_file(std::ifstream &);
    void remove_chars_from_string(std::string &, const std::vector<char> &);
    int extract_number_from_string(const std::string &, const size_t, const size_t);
    std::vector<std::string> split_string_by_character(const std::string &, char);
    bool arg_exists(const std::string &, const std::string &, const std::string &);
    std::string get_arg(const std::string &, const std::string &, const std::string &, const std::string &);
    void print_size_mismatch_error(const size_t, const size_t, const std::string &, const std::string &);
    bool bool_arg_converter(const std::string &);
    int random_int_between(const int, const int, std::minstd_rand &);
    double get_percent_error(const double, const double);

    template <typename T, typename S>
    void remove_key_subset_from_map(std::map<T, S> &map, const std::unordered_set<T> key_subset)
    {
        for (auto key : key_subset)
        {
            map.erase(key);
        }
    }

    template <typename T>
    std::vector<T> get_list_arg(const std::string &options_string,
                                const std::string &short_form,
                                const std::string &long_form,
                                const std::string &help_info,
                                const size_t expected_size,
                                const std::function<T(std::string)> &string_converter)
    {
        std::vector<T> list(expected_size);
        auto arg_string = get_arg(options_string, short_form, long_form, help_info);
        if (arg_string.empty())
        {
            throw std::invalid_argument("Size mismatch.");
        }
        else
        {
            auto string_list = split_string_by_character(arg_string, ',');
            if (string_list.size() != expected_size)
            {
                print_size_mismatch_error(expected_size, string_list.size(), short_form, long_form);
            }
            for (size_t i = 0; i < expected_size; i++)
            {
                list[i] = string_converter(string_list[i]);
            }
        }

        return list;
    }

    template <typename T>
    std::vector<T> get_list_arg(const std::string &options_string,
                                const std::string &short_form,
                                const std::string &long_form,
                                const std::string &help_info,
                                const size_t expected_size,
                                const T &default_value,
                                const std::function<T(std::string)> &string_converter)
    {
        try
        {
            return get_list_arg(options_string,
                                short_form,
                                long_form,
                                help_info,
                                expected_size,
                                string_converter);
        }
        catch (std::invalid_argument)
        {
            std::vector<T> list;
            list.resize(expected_size, default_value);
            return list;
        }
    }

    template <typename T>
    class CallWrapper
    {
    private:
        T temp;
        std::function<T()> func;

    public:
        CallWrapper(const std::function<T()> &func) : func{func} {}

        void call_without_return()
        {
            temp = std::move(func());
        }

        T &&return_and_destroy()
        {
            return std::move(temp);
        }
    };

    template <>
    class CallWrapper<void>
    {
    private:
        std::function<void()> func;

    public:
        CallWrapper(const std::function<void()> &func) : func{func} {}

        void call_without_return()
        {
            func();
        }

        void return_and_destroy() {}
    };

    template <typename T>
    T perform_func_and_print_execution_time(const std::function<T()> &func, const std::string &task_preamble, double *execution_time_ptr)
    {
        auto func_wrapper = utl::CallWrapper(func);

        using hrc = std::chrono::high_resolution_clock;
        using TimeSpanType = std::chrono::duration<double>;

        std::cout << task_preamble << "..." << std::endl;
        auto t1 = hrc::now();
        func_wrapper.call_without_return();
        auto t2 = hrc::now();
        auto time_span = std::chrono::duration_cast<TimeSpanType>(t2 - t1);
        auto execution_time = time_span.count();
        if (execution_time_ptr != nullptr)
        {
            *execution_time_ptr = execution_time;
        }
        std::cout << "Execution time: " << execution_time << " seconds." << std::endl;
        return func_wrapper.return_and_destroy();
    }

    template <typename T>
    T perform_func_and_print_execution_time(std::function<T()> func, const std::string &task_preamble)
    {
        return perform_func_and_print_execution_time(func, task_preamble, nullptr);
    }

    template <typename T>
    T perform_func_and_print_execution_time(std::function<T()> func, const char *task_preamble)
    {
        return perform_func_and_print_execution_time(func, std::string(task_preamble));
    }
}

#endif