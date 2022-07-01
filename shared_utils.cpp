enum class OperationType
{
    ADD,
    MUL,
};

template <typename T>
bool vector_contains_element(const std::vector<T> &vector, const T &element)
{
    return std::find(vector.begin(), vector.end(), element) != vector.end();
}

template <typename T, typename S>
bool unordered_map_contains_key(const std::unordered_map<T, S> &vector, const T &element)
{
    return std::find(vector.begin(), vector.end(), element) != vector.end();
}

template <typename T>
void remove_element_from_vector(std::vector<T> &vector, const T &element)
{
    auto op_position = std::find(vector.begin(), vector.end(), element);
    vector.erase(op_position);
}