#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

// enum class OperationType
// {
//     ADD,
//     MUL,
// };

struct Operation
{
    std::string type;
    std::vector<int> parent_ids;
};

template <typename T>
bool vector_contains_element(const std::vector<T> &vector, const T &element)
{
    return std::find(vector.begin(), vector.end(), element) != vector.end();
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