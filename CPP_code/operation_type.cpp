#include "operation_type.h"

OperationType::OperationType(){};
OperationType::OperationType(const Type type) : type{type} {};
OperationType::OperationType(const std::string &type_string)
{
    if (type_string == "ADD")
    {
        type = ADD;
    }
    else if (type_string == "SUB")
    {
        type = SUB;
    }
    else if (type_string == "MUL")
    {
        type = MUL;
    }
    else if (type_string == "BOOT")
    {
        type = BOOT;
    }
    else
    {
        std::cout << "Invalid operation type: " << type_string << std::endl;
        exit(1);
    }
};

std::string OperationType::to_string() const
{
    switch (type)
    {
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case MUL:
        return "MUL";
    case BOOT:
        return "BOOT";
    }
    return "";
};

// bool operator=(Type other_type) { type = other_type; }

// bool operator==(Type other_type) { return type == other_type; }
// bool operator==(OperationType other_class) { return type == other_class.type; }
// bool operator!=(Type other_type) { return !operator==(other_type); }
// bool operator!=(OperationType other_class) { return !operator==(other_class); };

OperationType::operator Type() const { return type; }