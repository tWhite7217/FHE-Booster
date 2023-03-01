#ifndef program_INCLUDED_
#define program_INCLUDED_

#include "bootstrap_segment.h"
#include "shared_utils.h"

#include <fstream>
#include <stdexcept>

class Program
{
public:
    struct ConstructorInput
    {
        std::string dag_filename;
        std::string segments_filename;
        std::string bootstrap_filename;
        std::string latency_filename;
        BootstrapMode b_mode;
    };

    Program(){};
    Program(const ConstructorInput &);

    OpVector::const_iterator begin() const;
    OpVector::const_iterator end() const;
    size_t size() const;

    OperationPtr get_operation_ptr_from_id(const size_t) const;
    int get_latency_of(const OperationType::Type) const;
    int get_maximum_slack() const;
    int get_maximum_num_segments() const;
    bool bootstrap_segments_are_satisfied() const;
    int find_unsatisfied_bootstrap_segment_index() const;

    void add_operation(const OperationPtr &);
    void set_bootstrap_segments(const std::vector<BootstrapSegment> &);

    void update_num_segments_for_every_operation();
    void update_ESTs_and_LSTs();
    void reset_bootstrap_set();
    void update_all_bootstrap_urgencies();

    void remove_unnecessary_bootstrap_pairs();
    void convert_segments_to_selective();

private:
    OpVector operations;
    std::vector<BootstrapSegment> bootstrap_segments;
    BootstrapMode mode;
    LatencyMap latencies =
        {{OperationType::ADD, 1},
         {OperationType::SUB, 1},
         {OperationType::MUL, 5},
         {OperationType::BOOT, 300}};

    bool no_segment_relies_on_bootstrap_pair(const OperationPtr &, const OperationPtr &);

    class FileParser;
    friend class FileWriter;
};

#include "file_parser.h"

#endif