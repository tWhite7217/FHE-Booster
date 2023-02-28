#include "engine_operation.hpp"

EngineOperation::EngineOperation(const std::string &in_op, const std::string &output_key, const std::string &in1, const std::string &in2)
{
    assert(output_key != "");
    this->output_key = output_key;

    inputs.clear();
    add_input(in1);
    add_input(in2);

    set_op_type(in_op);
}

void EngineOperation::add_input(const std::string &input_key)
{
    if (!input_key.empty())
    {
        bool is_ctxt = input_key.find('p') == std::string::npos;
        inputs.push_back({input_key, is_ctxt});
    }
}

void EngineOperation::set_op_type(const std::string &in_op)
{
    if (in_op == "MUL" || in_op == "ADD" || in_op == "SUB")
    {
        assert(inputs.size() == 2);
        bool is_ctxt1 = inputs[0].is_ctxt;
        bool is_ctxt2 = inputs[1].is_ctxt;
        if (is_ctxt1 & is_ctxt2)
        {
            op_type = in_op == "MUL" ? CC_MUL : (in_op == "ADD" ? CC_ADD : CC_SUB);
        }
        else if (is_ctxt1 ^ is_ctxt2)
        {
            auto sub_type = CP_SUB;
            if (is_ctxt2)
            {
                std::swap(inputs[0], inputs[1]);
                sub_type = PC_SUB;
            }
            op_type = in_op == "MUL" ? CP_MUL : (in_op == "ADD" ? CP_ADD : sub_type);
        }
        else
        {
            std::cout << "ERROR: Ptxt-ptxt ops are not supported!" << std::endl;
            std::cout << "Trace: " << in_op << " " << inputs[0].key << " "
                      << inputs[1].key << " " << output_key << std::endl;
            exit(-1);
        }
    }
    else if (in_op == "BOOT" || in_op == "INV")
    {
        assert(inputs.size() == 1);
        if (inputs[0].is_ctxt)
        {
            op_type = in_op == "BOOT" ? BOOT : INV;
        }
        else
        {
            std::cout << "ERROR: Invalid operation on ptxt!" << std::endl;
            std::cout << "Trace: " << in_op << " " << inputs[0].key << " "
                      << " " << output_key << std::endl;
            exit(-1);
        }
    }
    else
    {
        std::cout << "ERROR: Unsupported operation: " << in_op << "!" << std::endl;
        exit(-1);
    }
}

std::vector<EngineOpInput> EngineOperation::get_inputs() const
{
    return inputs;
}

std::string EngineOperation::get_output() const
{
    return output_key;
}

EngineOpType EngineOperation::get_op_type() const
{
    return op_type;
}

void EngineOperation::print() const
{
    std::cout << "Op: " << op_type << std::endl;
    std::cout << "Input(s): ";
    for (uint64_t i = 0; i < inputs.size(); i++)
    {
        std::cout << inputs[i].key << " ";
    }
    std::cout << std::endl;
    std::cout << "Output: " << output_key << std::endl;
}