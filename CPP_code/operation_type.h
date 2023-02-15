#ifndef operation_type_INCLUDED_
#define operation_type_INCLUDED_

#include <string>
#include <iostream>

class OperationType
{
public:
    enum Type
    {
        ADD,
        SUB,
        MUL,
        BOOT
    };

    static const size_t num_types_except_bootstrap = BOOT;

    OperationType();
    OperationType(const Type);
    OperationType(const std::string &);

    std::string to_string() const;

    // bool operator=(Type other_type);

    // bool operator==(Type );
    // bool operator==(OperationType);
    // bool operator!=(Type );
    // bool operator!=(OperationType);

    operator Type() const;

private:
    Type type;
};

#endif