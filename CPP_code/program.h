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
    bool has_unsatisfied_bootstrap_segments() const;
    void initialize_unsatisfied_segment_indexes();
    void initialize_num_segments_for_every_operation();
    void initialize_alive_segment_indexes();
    void initialize_operation_to_segments_map();
    std::vector<size_t> update_unsatisfied_segments_and_num_segments_for_every_operation();
    void update_alive_segments(const OperationPtr &, const std::vector<size_t> &);

    OperationPtr add_operation(const Operation &);
    void set_bootstrap_segments(const std::vector<BootstrapSegment> &);
    void set_boot_mode(const BootstrapMode);

    void update_slack_for_every_operation();
    void reset_bootstrap_set();
    void update_all_bootstrap_urgencies();

    void remove_unnecessary_bootstrap_pairs(size_t &, size_t &);

private:
    OpVector operations;
    std::vector<std::unique_ptr<Operation>> operation_ptrs;
    std::vector<BootstrapSegment> bootstrap_segments;
    std::unordered_set<size_t> unsatisfied_bootstrap_segment_indexes;
    std::unordered_set<size_t> alive_bootstrap_segment_indexes;
    std::unordered_map<OperationPtr, std::unordered_set<size_t>> segment_indexes_started_by_op;
    LatencyMap latencies =
        {{OperationType::ADD, 1},
         {OperationType::SUB, 1},
         {OperationType::MUL, 5},
         {OperationType::BOOT, 300}};
    BootstrapPairIndexesMap get_candidate_pairs_and_segment_indexes(size_t &);
    BootstrapMode mode = BootstrapMode::COMPLETE;
    bool no_segment_relies_on_bootstrap_pair(const BootstrapPair &, const std::unordered_set<size_t> &);

    class FileParser;
    friend class FileWriter;
};

#include "file_parser.h"

#endif