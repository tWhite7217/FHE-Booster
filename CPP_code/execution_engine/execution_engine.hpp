#include <map>
#include <cstdlib>
#include <ctime>
#include <ratio>
#include <chrono>
#include <mutex>
#include <memory>
#include <unordered_set>
#include <atomic>
#include <stdexcept>
#include "omp.h"

#include "parser.hpp"
#include "openfhe.h"

class ExecutionEngine
{
  using ContextType = lbcrypto::CryptoContext<lbcrypto::DCRTPoly>;
  using Ctxt = lbcrypto::Ciphertext<lbcrypto::DCRTPoly>;
  using Ptxt = lbcrypto::Plaintext;
  using KeyPair = lbcrypto::KeyPair<lbcrypto::DCRTPoly>;

public:
  ExecutionEngine(int, char **);
  void execute();
  void print_schedule() const;

private:
  const std::string help_info =
      R"(
Usage: ./execution_engine <sched_file> [<options>]" << std::endl;
  
<sched_file> should not include the \".sched\" extension.

Options:
  -l <int>, --num-levels=<int>
    The number of levels between bootstraps, also called the noise threshold. Defaults to 9.
  -r <float>, --rand-thresh=<float>
    The maximum value of randomly generated inputs. Defaults to 1.0.
  -m <mode>, --mode=<mode>
    The execution mode. There are three possible options, of which BOOSTER is the default.
      BOOSTER: Standard execution mode, with all operation types supported.
      PLAINTEXT: Performs the schedule in the plaintext domain, printing output values at the end.
      ALAP: BOOT operations not allowed. Bootstrapping is performed dynamically, as late as possible.
  -v, --verify
    Decrypts the results from the encrypted domain, and compares to expected values. Ignored if mode=PLAINTEXT.
  -b, --bootstrap-inputs
    Bootstraps the initial ciphertext inputs before performing the schedule.
  -o <string>, --output-suffix=<string>
    A file named \"<sched_file>_eval_time_<mode>_<string>.txt\" stores the evaluation time of the schedule.
  -s, --save-num-bootstraps
    A file named \"<sched_file>_num_bootstraps.txt\" stores the number of the bootstraps performed executing the schedule.)";

  CommandLineOptions options;

  ScheduleInfo sched_info;
  ExecMode mode;
  ContextType context;
  KeyPair key_pair;

  int level_to_bootstrap;

  std::map<std::string, Ctxt> ctxt_regs;
  std::map<std::string, Ptxt> ptxt_regs;
  std::map<std::string, double> validation_regs;
  std::map<std::string, std::shared_ptr<std::mutex>> reg_locks;
  std::map<std::string, std::shared_ptr<std::mutex>> dependence_locks;

  double execution_time;
  int num_bootstraps;

  void print_test_info() const;
  void print_options() const;
  void validate_results() const;

  void parse_schedule();
  void generate_data_structures();
  void execute_in_plaintext();
  void setup_crypto_context();
  void prepare_inputs();
  void execute_in_ciphertext();
  void verify_results();
  void write_execution_info_to_files();

  void parse_args(int, char **);
  void update_dependence_info(const EngineOpInput &, const std::string &);
  void handle_input_mutex(const std::string &);
  int execute_schedule();
  void execute_validation_schedule();
  void generate_random_inputs();
  void encrypt_inputs();
  void generate_reg_locks();
  void generate_dependence_locks();
  void lock_all_mutexes();
  void bootstrap_initial_inputs();
};