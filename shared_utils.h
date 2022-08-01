
#ifndef shared_utils_INCLUDED_
#define shared_utils_INCLUDED_

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <memory>

// enum class OperationType
// {
//     ADD,
//     MUL,
// };

const int bootstrapping_latency = 12;
const int addition_divider = 1;
const int bootstrapping_path_threshold = 3;

struct Operation;

using OperationPtr = std::shared_ptr<Operation>;

struct Operation
{
    std::string type;
    std::vector<OperationPtr> parent_ptrs;
    int id;
    std::vector<OperationPtr> child_ptrs;
    int start_time;
    int bootstrap_start_time = 0;
    int core_num = 0;
    int earliest_start_time;
    int latest_start_time;
    int rank;
};

using OperationList = std::vector<OperationPtr>;

OperationPtr get_operation_ptr_from_id(OperationList, int);

bool operations_bootstrap_on_same_core(OperationPtr, OperationPtr);

template <typename T>
bool vector_contains_element(const std::vector<T> &vector, const T &element)
{
    return std::find(vector.begin(), vector.end(), element) != vector.end();
}

template <typename T, typename S>
bool set_contains_element(const std::set<T, S> &set, const T &element)
{
    return set.find(element) != set.end();
}

template <typename T, typename S>
bool map_contains_key(const std::map<T, S> &map, const T &element)
{
    return map.find(element) != map.end();
}

template <typename T, typename S>
bool unordered_map_contains_key(const std::unordered_map<T, S> &map, const T &element)
{
    return map.find(element) != map.end();
}

template <typename T>
void remove_element_from_vector(std::vector<T> &vector, const T &element)
{
    auto op_position = std::find(vector.begin(), vector.end(), element);
    vector.erase(op_position);
}

template <typename T>
void remove_element_subset_from_vector(std::vector<T> &vector, const std::set<T> &element_subset)
{
    for (auto &element : element_subset)
    {
        remove_element_from_vector(vector, element);
    }
}

template <typename T, typename S>
S map_max_value(const std::map<T, S> &map)
{
    auto max_it = std::max_element(map.begin(), map.end(),
                                   [](const std::pair<T, S> &a, const std::pair<T, S> &b)
                                   { return a.second < b.second; });
    return max_it->second;
}

template <typename T, typename S>
void remove_key_subset_from_map(std::map<T, S> &map, const std::set<T> key_subset)
{
    for (auto key : key_subset)
    {
        map.erase(key);
    }
}

void remove_chars_from_string(std::string &, std::vector<char>);
int extract_number_from_string(std::string, size_t, size_t);

#endif