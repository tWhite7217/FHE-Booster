#include <string>
#include <vector>
#include <cassert>
#include <memory>
#include <iostream>

enum EngineOpType
{
    CC_MUL,
    CP_MUL,
    CC_ADD,
    CP_ADD,
    CC_SUB,
    CP_SUB,
    PC_SUB,
    INV,
    BOOT
};

struct EngineOpInput
{
    std::string key;
    bool is_ctxt;

    bool operator==(const EngineOpInput &other) const
    {
        return key == other.key;
    }

    struct hash
    {
        auto operator()(const EngineOpInput &eoi) const
        {
            return std::hash<std::string>()(eoi.key);
        }
    };
};

class EngineOperation
{
public:
    EngineOperation() {}
    EngineOperation(const std::string &in_op, const std::string &in_out, const std::string &in1, const std::string &in2);

    std::vector<EngineOpInput> get_inputs() const;
    std::string get_output() const;
    EngineOpType get_op_type() const;
    void print() const;

private:
    std::vector<EngineOpInput> inputs;
    std::string output_key;
    EngineOpType op_type;

    void add_input(const std::string &);
    void set_op_type(const std::string &in_op);
};

using EngineOperationPtr = std::shared_ptr<EngineOperation>;