#ifndef program_INCLUDED_
#define program_INCLUDED_

#include "bootstrap_segment.h"
#include "shared_utils.h"
#include "custom_ddg_format_parser.h"

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

    // std::vector<BootstrapSegment> get_bootstrap_segments();
    // BootstrapMode get_bootstrap_mode();

    OpVector::const_iterator begin() const;
    OpVector::const_iterator end() const;
    size_t size() const;

    OperationPtr get_operation_ptr_from_id(const int &);
    int get_latency_of(OperationType::Type);
    int get_maximum_slack();
    int get_maximum_num_segments();
    // void add_segment_index_info_to_operations();
    bool bootstrap_segments_are_satisfied();
    // bool bootstrap_segments_are_satisfied_for_selective_model();
    int find_unsatisfied_bootstrap_segment_index();

    void add_operation(const OperationPtr &);

    void update_num_segments_for_every_operation();
    void update_ESTs_and_LSTs();
    void reset_bootstrap_set();
    void update_all_bootstrap_urgencies();

    void remove_unnecessary_bootstrap_pairs();

    void write_bootstrapping_set_to_file(const std::string &);
    void write_lgr_info_to_file(const std::string &, int);

private:
    OpVector operations;
    std::vector<BootstrapSegment> bootstrap_segments;
    LatencyMap latencies;
    BootstrapMode mode;

    void write_bootstrapping_set_to_file(std::ofstream &);
    void write_bootstrapping_set_to_file_complete_mode(std::ofstream &);
    void write_bootstrapping_set_to_file_selective_mode(std::ofstream &);

    bool no_segment_relies_on_bootstrap_pair(const OperationPtr &, const OperationPtr &);
};

#endif