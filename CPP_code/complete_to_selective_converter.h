#include "program.h"

#include <iostream>

class CompleteToSelectiveConverter
{
public:
    CompleteToSelectiveConverter(const std::string &, const std::string &, const std::string &);
    void write_selective_lgr_file(const std::string &);

private:
    Program program;
};