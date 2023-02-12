#include "program.h"

#include <iostream>

class LimitedToSelectiveConverter
{
public:
    LimitedToSelectiveConverter(std::string, std::string, std::string, int);
    void write_selective_lgr_file(std::string);

private:
    Program program;
};