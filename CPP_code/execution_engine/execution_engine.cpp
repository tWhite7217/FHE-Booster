#include "map"
#include <cstdlib>
#include "omp.h"
#include <ctime>
#include <ratio>
#include <chrono>
#include <mutex>
#include <memory>
#include <unordered_set>
#include <atomic>
#include <stdexcept>

#include "../shared_utils.h"
#include "parser.hpp"
#include "openfhe.h"

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
    A file named \"<sched_file>_eval_time_<string>.txt\" stores the evaluation time of the schedule.
  -s, --save-num-bootstraps
    A file named \"<sched_file>_num_bootstraps.txt\" stores the number of the bootstraps performed executing the schedule.)";

namespace lbc = lbcrypto;
using ContextType = lbc::CryptoContext<lbc::DCRTPoly>;
using Ctxt = lbc::Ciphertext<lbc::DCRTPoly>;
using Ptxt = lbc::Plaintext;

struct ExecutionVariables
{
  std::map<std::string, Ctxt> e_regs;
  std::map<std::string, Ptxt> p_regs;
  std::map<std::string, double> v_regs;
  std::map<std::string, std::shared_ptr<std::mutex>> reg_locks;
  std::map<std::string, std::shared_ptr<std::mutex>> dep_locks;
};

void print_schedule(std::vector<std::queue<std::shared_ptr<Node>>> schedule)
{
  for (uint64_t i = 0; i < schedule.size(); i++)
  {
    if (schedule[i].empty())
    {
      continue;
    }
    std::cout << "=== Nodes for Worker " << i << " ===" << std::endl;
    while (!schedule[i].empty())
    {
      schedule[i].front()->print_node();
      schedule[i].pop();
    }
  }
  return;
}

bool isCtxt(const std::string &input_index)
{
  return input_index.find('p') == std::string::npos;
}

std::pair<std::string, std::string> get_input_indices(const std::queue<std::shared_ptr<Node>> &core_schedule)
{
  std::vector<std::string> inputs = core_schedule.front()->get_inputs();
  switch (core_schedule.front()->get_op())
  {
  case CMUL:
  case CADD:
  case CSUB:
    return isCtxt(inputs[0]) ? std::pair(inputs[0], inputs[1]) : std::pair(inputs[1], inputs[0]);
    break;
  case EMUL:
  case EADD:
  case ESUB:
    return std::pair(inputs[0], inputs[1]);
    break;
  case EINV:
  case BOOT:
    return std::pair(inputs[0], "");
    break;
  default:
    std::cout << "Invalid Instruction! Exiting..." << std::endl;
    exit(-1);
  }
}

void handle_input_mutex(std::map<std::string, std::shared_ptr<std::mutex>> &reg_locks, const std::string &input_index)
{
  if (reg_locks.count(input_index))
  {
    reg_locks[input_index]->lock();
    reg_locks[input_index]->unlock();
  }
}

void update_dependence_info(const std::string &input_index,
                            const std::string &output_index,
                            ScheduleInfo &sched_info,
                            ExecutionVariables &vars)
{
  vars.dep_locks[input_index]->lock();
  sched_info.dependent_outputs[input_index].erase(output_index);
  if (sched_info.dependent_outputs[input_index].empty())
  {
    if (isCtxt(input_index))
    {
      vars.e_regs.erase(input_index);
      vars.reg_locks.erase(input_index);
    }
    else
    {
      vars.p_regs.erase(input_index);
    }
  }
  vars.dep_locks[input_index]->unlock();
}

int execute_schedule(ScheduleInfo sched_info,
                     ExecutionVariables &vars,
                     ContextType &context,
                     const ExecMode &mode,
                     const uint32_t &level_to_bootstrap)
{
  const bool ALAP_mode = (mode == ALAP);
  std::atomic<int> bootstrap_counter = 0;
  auto &enc_regs = vars.e_regs;
  auto &ptxt_regs = vars.p_regs;
  auto &reg_locks = vars.reg_locks;
#pragma omp parallel for
  for (uint64_t i = 0; i < sched_info.circuit.size(); i++)
  {
    auto &core_schedule = sched_info.circuit[i];
    while (!core_schedule.empty())
    {
      auto output_index = core_schedule.front()->get_output();
      auto input_indices = get_input_indices(core_schedule);
      auto input_index1 = input_indices.first;
      auto input_index2 = input_indices.second;
      handle_input_mutex(reg_locks, input_index1);
      handle_input_mutex(reg_locks, input_index2);
      switch (core_schedule.front()->get_op())
      {
      case CMUL:
        enc_regs[output_index] =
            context->EvalMult(enc_regs[input_index1], ptxt_regs[input_index2]);
        context->ModReduceInPlace(enc_regs[output_index]);
        break;
      case EMUL:
        enc_regs[output_index] =
            context->EvalMult(enc_regs[input_index1], enc_regs[input_index2]);
        context->ModReduceInPlace(enc_regs[output_index]);
        break;
      case CADD:
        enc_regs[output_index] =
            context->EvalAdd(enc_regs[input_index1], ptxt_regs[input_index2]);
        break;
      case EADD:
        enc_regs[output_index] =
            context->EvalAdd(enc_regs[input_index1], enc_regs[input_index2]);
        break;
      case CSUB:
        enc_regs[output_index] =
            context->EvalSub(enc_regs[input_index1], ptxt_regs[input_index2]);
        if (isCtxt(input_index2))
        {
          enc_regs[output_index] = context->EvalNegate(enc_regs[output_index]);
        }
        break;
      case ESUB:
        enc_regs[output_index] =
            context->EvalSub(enc_regs[input_index1], enc_regs[input_index2]);
        break;
      case EINV:
        enc_regs[output_index] = context->EvalNegate(enc_regs[input_index1]);
        break;
      case BOOT:
        enc_regs[output_index] = context->EvalBootstrap(enc_regs[input_index1]);
        bootstrap_counter++;
        break;
      default:
        std::cout << "Invalid Instruction! Exiting..." << std::endl;
        exit(-1);
      }
      if (ALAP_mode &&
          sched_info.bootstrap_candidates.count(output_index) &&
          enc_regs[output_index]->GetLevel() >= level_to_bootstrap)
      {
        enc_regs[output_index] = context->EvalBootstrap(enc_regs[output_index]);
        bootstrap_counter++;
      }

      reg_locks[output_index]->unlock();

      update_dependence_info(input_index1, output_index, sched_info, vars);
      if (input_index2 != "")
      {
        update_dependence_info(input_index2, output_index, sched_info, vars);
      }

      // std::cout << output_index << " level: " << enc_regs[output_index]->GetLevel() << std::endl;
      core_schedule.pop();
    }
  }
  return bootstrap_counter;
}

void execute_validation_schedule(ScheduleInfo sched_info,
                                 ExecutionVariables &vars)
{
  auto &validation_regs = vars.v_regs;
  auto &reg_locks = vars.reg_locks;
#pragma omp parallel for
  for (uint64_t i = 0; i < sched_info.circuit.size(); i++)
  {
    auto &core_schedule = sched_info.circuit[i];
    while (!core_schedule.empty())
    {
      auto output_index = core_schedule.front()->get_output();
      auto inputs = core_schedule.front()->get_inputs();
      auto input_index1 = inputs[0];
      std::string input_index2 = "";
      if (inputs.size() == 2)
      {
        input_index2 = inputs[1];
        handle_input_mutex(reg_locks, input_index2);
      }
      handle_input_mutex(reg_locks, input_index1);
      switch (core_schedule.front()->get_op())
      {
      case CMUL:
      case EMUL:
        validation_regs[output_index] =
            validation_regs[input_index1] * validation_regs[input_index2];
        break;
      case CADD:
      case EADD:
        validation_regs[output_index] =
            validation_regs[input_index1] + validation_regs[input_index2];
        break;
      case CSUB:
      case ESUB:
        validation_regs[output_index] =
            validation_regs[input_index1] - validation_regs[input_index2];
        break;
      case EINV:
        validation_regs[output_index] = -validation_regs[input_index1];
        break;
      case BOOT:
        validation_regs[output_index] = validation_regs[input_index1];
        break;
      default:
        std::cout << "Invalid Instruction! Exiting..." << std::endl;
        exit(-1);
      }
      reg_locks[output_index]->unlock();
      core_schedule.pop();
    }
  }
  return;
}

double get_percent_error(double experimental, double expected)
{
  if (expected == 0)
  {
    return 999;
    // return std::abs(experimental) < 0.000000001;
  }
  return std::abs(experimental - expected) / expected * 100;
}

void validate_results(const ExecutionVariables &vars,
                      lbc::PrivateKey<lbc::DCRTPoly> &private_key,
                      ContextType &context)
{
  const auto &enc_regs = vars.e_regs;
  const auto &validation_regs = vars.v_regs;
  for (const auto &[key, ctxt] : enc_regs)
  {
    Ptxt tmp_ptxt;
    context->Decrypt(private_key, ctxt, &tmp_ptxt);
    auto decrypted_val = tmp_ptxt->GetRealPackedValue()[0];

    auto percent_error = get_percent_error(decrypted_val, validation_regs.at(key));
    if (percent_error > 0.5)
    {
      std::cout << key << ": FHE result: " << decrypted_val << ", expected: " << validation_regs.at(key) << ", error: " << percent_error << "%" << std::endl;
    }
  }
}

void generate_random_inputs(const ScheduleInfo &sched_info,
                            ExecutionVariables &vars,
                            double rand_thresh)
{
  auto &validation_regs = vars.v_regs;
  for (const auto &key : sched_info.initial_inputs)
  {
    // validation_regs[key] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX / rand_thresh);
    validation_regs[key] = rand_thresh;
  }
}

void encrypt_inputs(ExecutionVariables &vars,
                    lbc::PublicKey<lbc::DCRTPoly> &pub_key,
                    ContextType &context)
{
  for (const auto &[key, value] : vars.v_regs)
  {
    std::vector<double> tmp_vec;
    tmp_vec.push_back(value);
    auto tmp_ptxt = context->MakeCKKSPackedPlaintext(tmp_vec);

    if (isCtxt(key))
    {
      auto tmp = context->Encrypt(pub_key, tmp_ptxt);
      vars.e_regs[key] = tmp;
    }
    else
    {
      vars.p_regs[key] = tmp_ptxt;
    }
  }
}

std::map<std::string, std::shared_ptr<std::mutex>> get_reg_locks(ScheduleInfo sched_info)
{
  std::map<std::string, std::shared_ptr<std::mutex>> reg_locks;
  for (uint64_t i = 0; i < sched_info.circuit.size(); i++)
  {
    auto &core_schedule = sched_info.circuit[i];
    while (!core_schedule.empty())
    {
      auto output_index = core_schedule.front()->get_output();
      reg_locks.emplace(output_index, new std::mutex);
      reg_locks[output_index]->lock();
      core_schedule.pop();
    }
  }
  return reg_locks;
}

std::map<std::string, std::shared_ptr<std::mutex>> get_dep_locks(const ScheduleInfo &sched_info)
{
  std::map<std::string, std::shared_ptr<std::mutex>> dependence_locks;
  for (const auto &[input_index, _] : sched_info.dependent_outputs)
  {
    dependence_locks.emplace(input_index, new std::mutex);
  }
  return dependence_locks;
}

void lock_all_mutexes(std::map<std::string, std::shared_ptr<std::mutex>> &reg_locks)
{
  for (auto &[key, mutex] : reg_locks)
  {
    mutex->lock();
  }
}

void bootstrap_initial_inputs(std::map<std::string, Ctxt> &enc_regs,
                              ContextType &context)
{
#pragma omp parallel for
  for (int i = 0; i < enc_regs.size(); i++)
  {
    auto it = enc_regs.begin();
    advance(it, i);
    auto &[_, ctxt] = *it;
    ctxt = context->EvalBootstrap(ctxt);
  }
}

void print_test_info(const std::map<std::string, double> &validation_regs)
{
  int good_count = 0;
  int bad_count = 0;
  for (auto &[key, value] : validation_regs)
  {
    if (value > 0.000001 && value < 10000)
    {
      good_count++;
    }
    else
    {
      bad_count++;
      std::cout << key << ": " << value << std::endl;
    }
  }
  std::cout << "good values: " << good_count << std::endl;
  std::cout << "bad values: " << bad_count << std::endl;
}

CommandLineOptions parse_args(int argc, char **argv)
{
  CommandLineOptions options;

  std::string sched_file = argv[1];
  options.input_filename = sched_file + ".sched";

  std::string options_string;
  for (auto i = 2; i < argc; i++)
  {
    options_string += std::string(argv[i]) + " ";
  }

  auto num_levels_string = get_arg(options_string, "-l", "--num-levels", help_info);
  if (!num_levels_string.empty())
  {
    options.num_levels = stoi(num_levels_string);
  }

  auto rand_thresh_string = get_arg(options_string, "-r", "--rand-thresh", help_info);
  if (!rand_thresh_string.empty())
  {
    options.rand_thresh = stod(rand_thresh_string);
  }

  options.mode_string = get_arg(options_string, "-m", "--mode", help_info);
  if (options.mode_string.empty())
  {
    options.mode_string = "BOOSTER";
    options.mode = BOOSTER;
  }
  else if (options.mode_string == "BOOSTER")
  {
    options.mode = BOOSTER;
  }
  else if (options.mode_string == "ALAP")
  {
    options.mode = ALAP;
  }
  else if (options.mode_string == "PLAINTEXT")
  {
    options.mode = PLAINTEXT;
  }
  else
  {
    throw std::invalid_argument(options.mode_string + "is not a valid execution mode");
  }

  if (arg_exists(options_string, "-v", "--verify"))
  {
    options.verify_results = true;
  }

  if (arg_exists(options_string, "-b", "--bootstrap-inputs"))
  {
    options.bootstrap_inputs = true;
  }

  auto output_suffix = get_arg(options_string, "-o", "--output-suffix", help_info);
  if (!output_suffix.empty())
  {
    options.eval_time_filename = sched_file + "_eval_time_" + options.mode_string + "_" + output_suffix + ".txt";
  }

  if (arg_exists(options_string, "-s", "--save-num-bootstraps"))
  {
    options.num_bootstraps_filename = sched_file + "_num_bootstraps_" + options.mode_string + ".txt";
  }

  return options;
}

void print_options(CommandLineOptions options)
{
  std::cout << "FHE-Runner using the following options." << std::endl;
  std::cout << "num_levels: " << options.num_levels << std::endl;
  std::cout << "rand_thresh: " << options.rand_thresh << std::endl;
  std::cout << "mode: " << options.mode_string << std::endl;
  std::cout << "verify_results: " << (options.verify_results ? "yes" : "no") << std::endl;
  std::cout << "bootstrap_inputs: " << (options.bootstrap_inputs ? "yes" : "no") << std::endl;
  std::cout << "input_filename: " << options.input_filename << std::endl;
  std::cout << "eval_time_filename: " << options.eval_time_filename << std::endl;
  std::cout << "num_bootstraps_filename: " << options.num_bootstraps_filename << std::endl;
}

int main(int argc, char **argv)
{
  CommandLineOptions options;
  if (argc < 2)
  {
    std::cout << help_info << std::endl;
    exit(0);
  }
  else
  {
    try
    {
      options = parse_args(argc, argv);
      print_options(options);
    }
    catch (...)
    {
      std::cout << help_info << std::endl;
      exit(-1);
    }
  }

  std::cout << "Parsing schedule..." << std::endl;
  auto sched_info = parse_schedule(options);
  std::cout << "Done." << std::endl;

  omp_set_num_threads(sched_info.circuit.size());

  ExecutionVariables vars;
  srand(time(NULL));
  std::cout << "Generating random inputs..." << std::endl;
  generate_random_inputs(sched_info, vars, options.rand_thresh);
  std::cout << "Done." << std::endl;

  std::cout << "Generating mutexes..." << std::endl;
  vars.reg_locks = get_reg_locks(sched_info);
  vars.dep_locks = get_dep_locks(sched_info);
  std::cout << "Done." << std::endl;

  if (options.mode == PLAINTEXT || options.verify_results)
  {
    std::cout << "Executing in plaintext..." << std::endl;
    execute_validation_schedule(sched_info, vars);
    std::cout << "Done." << std::endl;
    lock_all_mutexes(vars.reg_locks);
  }

  if (options.mode == PLAINTEXT)
  {
    print_test_info(vars.v_regs);
  }
  else
  {
    std::cout << "Setting up crypto context..." << std::endl;
    lbc::CCParams<lbc::CryptoContextCKKSRNS> parameters;
    SecretKeyDist secretKeyDist = UNIFORM_TERNARY;
    parameters.SetSecretKeyDist(secretKeyDist);
    // parameters.SetSecurityLevel(HEStd_128_classic);
    parameters.SetSecurityLevel(lbc::HEStd_NotSet);
    parameters.SetRingDim(1 << 12);

#if NATIVEINT == 128
    ScalingTechnique rescaleTech = FIXEDAUTO;
    usint dcrtBits = 78;
    usint firstMod = 89;
#else
    ScalingTechnique rescaleTech = FLEXIBLEAUTO;
    usint dcrtBits = 59;
    usint firstMod = 60;
#endif

    parameters.SetScalingModSize(dcrtBits);
    parameters.SetScalingTechnique(rescaleTech);
    parameters.SetFirstModSize(firstMod);

    std::vector<uint32_t> levelBudget = {4, 4};
    uint32_t approxBootstrapDepth = 8;

    auto ctxt_level_after_bootstrap = lbc::FHECKKSRNS::GetBootstrapDepth(approxBootstrapDepth, levelBudget, secretKeyDist);
    // std::cout << "ctxt_level_after_bootstrap: " << ctxt_level_after_bootstrap << std::endl;
    uint32_t level_to_bootstrap = ctxt_level_after_bootstrap + options.num_levels;
    usint depth = level_to_bootstrap + 2;
    parameters.SetMultiplicativeDepth(depth);

    ContextType cryptoContext = GenCryptoContext(parameters);

    cryptoContext->Enable(PKE);
    cryptoContext->Enable(KEYSWITCH);
    cryptoContext->Enable(LEVELEDSHE);
    cryptoContext->Enable(ADVANCEDSHE);
    cryptoContext->Enable(FHE);

    usint ringDim = cryptoContext->GetRingDimension();
    usint numSlots = ringDim / 2;

    cryptoContext->EvalBootstrapSetup(levelBudget);

    auto keyPair = cryptoContext->KeyGen();
    cryptoContext->EvalMultKeyGen(keyPair.secretKey);
    cryptoContext->EvalBootstrapKeyGen(keyPair.secretKey, numSlots);
    std::cout << "Done." << std::endl;

    std::cout << "Encrypting inputs..." << std::endl;
    encrypt_inputs(vars, keyPair.publicKey, cryptoContext);
    std::cout << "Done." << std::endl;

    using hrc = std::chrono::high_resolution_clock;
    using TimeSpanType = std::chrono::duration<double>;
    if (options.bootstrap_inputs)
    {
      std::cout << "Bootstrapping " << sched_info.initial_inputs.size() << " inputs..." << std::endl;
      auto t1 = hrc::now();
      bootstrap_initial_inputs(vars.e_regs, cryptoContext);
      auto t2 = hrc::now();
      std::cout << "Done." << std::endl;
      auto time_span = std::chrono::duration_cast<TimeSpanType>(t2 - t1);
      std::cout << "Bootstrapping Time: " << time_span.count() << " seconds." << std::endl;
    }

    std::cout << "Executing in ciphertext..." << std::endl;
    auto t1 = hrc::now();
    int num_bootstraps = execute_schedule(sched_info, vars, cryptoContext, options.mode, level_to_bootstrap);
    auto t2 = hrc::now();
    std::cout << "Done." << std::endl;
    auto time_span = std::chrono::duration_cast<TimeSpanType>(t2 - t1);
    std::cout << "Eval Time: " << time_span.count() << " seconds." << std::endl;
    std::cout << "Number of bootstrapping operations: " << num_bootstraps << "." << std::endl;

    if (options.verify_results)
    {
      std::cout << "Comparing ctxt results against ptxt results..." << std::endl;
      validate_results(vars, keyPair.secretKey, cryptoContext);
      std::cout << "Done." << std::endl;
    }

    if (!options.eval_time_filename.empty())
    {
      std::ofstream time_file(options.eval_time_filename);
      time_file << time_span.count() << std::endl;
      time_file.close();
    }
    if (!options.num_bootstraps_filename.empty())
    {
      std::ofstream bootstrap_file(options.num_bootstraps_filename);
      bootstrap_file << num_bootstraps << std::endl;
      bootstrap_file.close();
    }
  }

  return 0;
}