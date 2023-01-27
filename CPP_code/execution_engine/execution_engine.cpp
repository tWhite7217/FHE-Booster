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

#include "parser.hpp"
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

struct ExecutionVariables
{
  std::map<string, Ciphertext<DCRTPoly>> e_regs;
  std::map<string, Plaintext> p_regs;
  std::map<string, double> v_regs;
  std::map<string, std::shared_ptr<std::mutex>> reg_locks;
  std::map<string, std::shared_ptr<std::mutex>> dep_locks;
};

void print_schedule(vector<queue<Node *>> schedule)
{
  for (uint64_t i = 0; i < schedule.size(); i++)
  {
    if (schedule[i].empty())
    {
      continue;
    }
    cout << "=== Nodes for Worker " << i << " ===" << endl;
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

std::pair<string, string> get_input_indices(const std::queue<Node *> &core_schedule)
{
  std::vector<string> inputs = core_schedule.front()->get_inputs();
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
    cout << "Invalid Instruction! Exiting..." << endl;
    exit(-1);
  }
}

void handle_input_mutex(std::map<string, std::shared_ptr<std::mutex>> &reg_locks, const string &input_index)
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
                     CryptoContext<DCRTPoly> &context,
                     const bool &do_T2_style_bootstrapping,
                     const uint32_t &level_to_bootstrap)
{
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
        cout << "Invalid Instruction! Exiting..." << endl;
        exit(-1);
      }
      if (do_T2_style_bootstrapping &&
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
      string input_index2 = "";
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
        cout << "Invalid Instruction! Exiting..." << endl;
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
                      PrivateKey<DCRTPoly> &private_key,
                      CryptoContext<DCRTPoly> &context)
{
  const auto &enc_regs = vars.e_regs;
  const auto &validation_regs = vars.v_regs;
  for (const auto &[key, ctxt] : enc_regs)
  {
    Plaintext tmp_ptxt;
    context->Decrypt(private_key, ctxt, &tmp_ptxt);
    auto decrypted_val = tmp_ptxt->GetRealPackedValue()[0];

    auto percent_error = get_percent_error(decrypted_val, validation_regs.at(key));
    if (percent_error > 0.5)
    {
      std::cout << key << ": FHE result: " << decrypted_val << ", expected: " << validation_regs.at(key) << ", error: " << percent_error << "%" << std::endl;
    }
  }
}

void gen_random_vals(const ScheduleInfo &sched_info,
                     ExecutionVariables &vars,
                     PublicKey<DCRTPoly> &pub_key,
                     CryptoContext<DCRTPoly> &context,
                     double rand_thresh)
{
  auto &enc_regs = vars.e_regs;
  auto &ptxt_regs = vars.p_regs;
  auto &validation_regs = vars.v_regs;
  for (const auto &key : sched_info.initial_inputs)
  {
    bool is_ctxt = key.find('p') == std::string::npos;
    if (is_ctxt)
    {
      vector<double> tmp_vec;
      // tmp_vec.push_back(static_cast <double> (rand()) / static_cast <double> (RAND_MAX/rand_thresh));
      tmp_vec.push_back(rand_thresh);
      auto tmp_ptxt = context->MakeCKKSPackedPlaintext(tmp_vec);
      auto tmp = context->Encrypt(pub_key, tmp_ptxt);
      enc_regs[key] = tmp;
      validation_regs[key] = tmp_vec[0];
    }
    else
    {
      vector<double> tmp_vec;
      // tmp_vec.push_back(static_cast <double> (rand()) / static_cast <double> (RAND_MAX/rand_thresh));
      tmp_vec.push_back(rand_thresh);
      auto tmp_ptxt = context->MakeCKKSPackedPlaintext(tmp_vec);
      ptxt_regs[key] = tmp_ptxt;
      validation_regs[key] = tmp_vec[0];
    }
  }
  return;
}

std::map<string, std::shared_ptr<std::mutex>> get_reg_locks(ScheduleInfo sched_info)
{
  std::map<string, std::shared_ptr<std::mutex>> reg_locks;
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

std::map<string, std::shared_ptr<std::mutex>> get_dep_locks(const ScheduleInfo &sched_info)
{
  std::map<string, std::shared_ptr<std::mutex>> dependence_locks;
  for (const auto &[input_index, _] : sched_info.dependent_outputs)
  {
    dependence_locks.emplace(input_index, new std::mutex);
  }
  return dependence_locks;
}

void lock_all_mutexes(std::map<string, std::shared_ptr<std::mutex>> &reg_locks)
{
  for (auto &[key, mutex] : reg_locks)
  {
    mutex->lock();
  }
}

void bootstrap_initial_inputs(std::map<string, Ciphertext<DCRTPoly>> &enc_regs,
                              CryptoContext<DCRTPoly> &context)
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

void print_test_info(const std::map<string, double> &validation_regs)
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

int main(int argc, char **argv)
{

  int num_workers = 1;
  int num_levels = 4;
  double rand_thresh = 1.0;
  bool do_T2_style_bootstrapping = false;
  bool test_mode = false;
  bool debug_mode = false;
  string sched_file = "";
  string filename = "";
  string eval_time_filename = "";
  string num_bootstraps_filename = "";
  int output_num = 0;
  if (argc != 7)
  {
    cout << "Usage: ./execution_engine [sched_file] [num_threads] [num_levels] [rand_thresh] [mode (T2, SCHED, TEST, DEBUG)] [output_num]" << endl;
    exit(0);
  }
  else
  {
    try
    {
      num_workers = atoi(argv[2]);
      num_levels = atoi(argv[3]);
      rand_thresh = atof(argv[4]);
      do_T2_style_bootstrapping = (std::string(argv[5]) == "T2");
      test_mode = (std::string(argv[5]) == "TEST");
      debug_mode = (std::string(argv[5]) == "DEBUG");
      output_num = atoi(argv[6]);
    }
    catch (...)
    {
      cout << "Invalid arguments, using defaults." << endl;
      num_workers = 1;
      num_levels = 4;
    }
    sched_file = argv[1];
    filename = sched_file + ".sched";
    eval_time_filename = sched_file + "_eval_time_" + argv[5] + "_" + std::to_string(output_num) + ".txt";
    num_bootstraps_filename = sched_file + "_num_bootstraps_" + argv[5] + ".txt";
  }

  // Set up crypto context
  CCParams<CryptoContextCKKSRNS> parameters;
  SecretKeyDist secretKeyDist = UNIFORM_TERNARY;
  parameters.SetSecretKeyDist(secretKeyDist);
  // parameters.SetSecurityLevel(HEStd_128_classic);
  parameters.SetSecurityLevel(HEStd_NotSet);
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

  auto ctxt_level_after_bootstrap = FHECKKSRNS::GetBootstrapDepth(approxBootstrapDepth, levelBudget, secretKeyDist);
  // std::cout << "ctxt_level_after_bootstrap: " << ctxt_level_after_bootstrap << std::endl;
  uint32_t level_to_bootstrap = ctxt_level_after_bootstrap + num_levels;
  usint depth = level_to_bootstrap + 2;
  // usint depth = level_to_bootstrap + 1;
  // usint depth = level_to_bootstrap;
  parameters.SetMultiplicativeDepth(depth);

  CryptoContext<DCRTPoly> cryptoContext = GenCryptoContext(parameters);

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

  cout << "Parsing schedule..." << endl;
  auto sched_info = parse_schedule(filename, num_workers, do_T2_style_bootstrapping);
  cout << "Done." << endl;
  omp_set_num_threads(sched_info.circuit.size());
  ExecutionVariables vars;
  duration<double> time_span;
  srand(time(NULL));
  cout << "Generating random inputs..." << endl;
  gen_random_vals(sched_info, vars, keyPair.publicKey, cryptoContext, rand_thresh);
  cout << "Done." << endl;
  if (!test_mode)
  {
    // if (do_T2_style_bootstrapping) {
    cout << "Bootstrapping " << sched_info.initial_inputs.size() << " inputs..." << endl;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    bootstrap_initial_inputs(vars.e_regs, cryptoContext);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    cout << "Done." << endl;
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "Bootstrapping Time: " << time_span.count() << " seconds." << endl;
    // }
  }

  vars.reg_locks = get_reg_locks(sched_info);
  vars.dep_locks = get_dep_locks(sched_info);

  int num_bootstraps;
  if (!test_mode)
  {
    cout << "Executing in ciphertext..." << endl;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    num_bootstraps = execute_schedule(sched_info, vars, cryptoContext, do_T2_style_bootstrapping, level_to_bootstrap);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    cout << "Done." << endl;
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "Eval Time: " << time_span.count() << " seconds." << endl;
    cout << "Number of bootstrapping operations: " << num_bootstraps << "." << endl;

    lock_all_mutexes(vars.reg_locks);
  }
  if (debug_mode)
  {
    cout << "Executing in plaintext..." << endl;
    execute_validation_schedule(sched_info, vars);
    cout << "Done." << endl;
  }
  if (!test_mode)
  {
    if (debug_mode)
    {
      cout << "Comparing ctxt results against ptxt results..." << endl;
      validate_results(vars, keyPair.secretKey, cryptoContext);
      cout << "Done." << endl;
    }

    ofstream time_file(eval_time_filename);
    time_file << time_span.count() << endl;
    time_file.close();
    ofstream bootstrap_file(num_bootstraps_filename);
    bootstrap_file << num_bootstraps << endl;
    bootstrap_file.close();
  }
  else
  {
    print_test_info(vars.v_regs);
  }

  return 0;
}