// Generated by Flexc++ V2.07.07 on Tue, 12 Jul 2022 14:29:54 -46045829

#ifndef LGRParser_H_INCLUDED_
#define LGRParser_H_INCLUDED_

// $insert baseclass_h
#include "LGRParserbase.h"

#include <map>
#include <functional>
#include "shared_utils.h"

// $insert classHead
class LGRParser : public LGRParserBase
{
public:
    explicit LGRParser(std::istream &in = std::cin,
                       std::ostream &out = std::cout);

    LGRParser(std::string const &infile, std::string const &outfile);

    // $insert lexFunctionDecl
    int lex();

    // My public members
    std::vector<OperationPtr> bootstrapped_operations;

    int max_finish_time = 0;
    bool used_bootstrap_limited_model = false;
    bool used_selective_model = false;

    void set_operations(OperationList &);
    bool operation_is_bootstrapped(OperationPtr);
    bool operation_is_bootstrapped(OperationPtr, OperationPtr);
    bool operations_bootstrap_on_same_core(int, int);

private:
    int lex_();
    int executeAction_(size_t ruleNr);

    void print();
    void preCode(); // re-implement this function for code that must
                    // be exec'ed before the patternmatching starts

    void postCode(PostEnum_ type);
    // re-implement this function for code that must
    // be exec'ed after the rules's actions.

    // My private members
    OperationList operations;

    std::function<bool(int)> get_checker_for_selective_model(OperationPtr, OperationPtr);
    OperationPtr get_first_operation_ptr(std::string);
    OperationPtr get_second_operation_ptr(std::string);
    int get_first_operation_id(std::string);
    int get_second_operation_id(std::string);
    int get_core_num(std::string);
    int get_time(std::string);
};

// $insert scannerConstructors
inline LGRParser::LGRParser(std::istream &in, std::ostream &out)
    : LGRParserBase(in, out)
{
}

inline LGRParser::LGRParser(std::string const &infile, std::string const &outfile)
    : LGRParserBase(infile, outfile)
{
}

// $insert inlineLexFunction
inline int LGRParser::lex()
{
    return lex_();
}

inline void LGRParser::preCode()
{
    // optionally replace by your own code
}

inline void LGRParser::postCode([[maybe_unused]] PostEnum_ type)
{
    // optionally replace by your own code
}

inline void LGRParser::print()
{
    print_();
}

#endif // LGRParser_H_INCLUDED_
