#include "program.h"
#include "LGRParser.h"

#include <iostream>

class LimitedToSelectiveConverter
{
public:
    LimitedToSelectiveConverter(std::string, std::string, std::string, int);
    void remove_unnecessary_bootstrapped_results();
    void write_selective_lgr_file(std::string);

private:
    Program program;
};