#include "program.h"

#include <iostream>

class CompleteToSelectiveConverter
{
public:
    CompleteToSelectiveConverter(std::string, std::string, std::string, int);
    void write_selective_lgr_file(std::string);

private:
    Program program;
};