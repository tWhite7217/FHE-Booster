#include "map"
#include <cstdlib>
#include "omp.h"
#include <ctime>
#include <ratio>
#include <chrono>
#include <mutex>
#include <memory>

#include "parser.hpp"
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

void print_schedule(vector<queue<Node*>> schedule) {
  for (uint64_t i = 0; i < schedule.size(); i++) {
    if (schedule[i].empty()) {
      continue;
    }
    cout << "=== Nodes for Worker " << i << " ===" << endl;
    while (!schedule[i].empty()) {
      schedule[i].front()->print_node();
      schedule[i].pop();
    }
  }
  return;
}

bool whichPtxt(const std::queue<Node *> &core_schedule) {
  return core_schedule.front()->get_inputs()[0].find('p') == std::string::npos;
}

std::pair<string, string> get_input_indices(const std::queue<Node *> &core_schedule) {
  std::vector<string> inputs = core_schedule.front()->get_inputs();
  switch(core_schedule.front()->get_op()) {
    case CMUL:
    case CADD:
    case CSUB:
      return whichPtxt(core_schedule) ? 
        std::pair(inputs[0], inputs[1]) :
        std::pair(inputs[1], inputs[0]);
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

void handle_input_mutex(std::map<string, std::shared_ptr<std::mutex>>& reg_locks, const string &input_index) {
      if (reg_locks.count(input_index)) {
        reg_locks[input_index]->lock();
        reg_locks[input_index]->unlock();
      }
}

void execute_schedule(std::map<string, Ciphertext<DCRTPoly>>& enc_regs, 
                     std::map<string, Plaintext>& ptxt_regs,
                     std::map<string, std::shared_ptr<std::mutex>>& reg_locks,
                     vector<queue<Node*>> schedule, PublicKey<DCRTPoly>& pub_key, 
                     CryptoContext<DCRTPoly>& context) {
  omp_set_num_threads(schedule.size());
  #pragma omp parallel for
  for (uint64_t i = 0; i < schedule.size(); i++) {
    auto &core_schedule = schedule[i];
    std::pair<Ciphertext<DCRTPoly>, Plaintext> indices;
    while(!core_schedule.empty()) {
      auto output_index = core_schedule.front()->get_output();
      auto input_indices = get_input_indices(core_schedule);
      auto input_index1 = input_indices.first;
      auto input_index2 = input_indices.second;
      handle_input_mutex(reg_locks, input_index1);
      handle_input_mutex(reg_locks, input_index2);
      switch(core_schedule.front()->get_op()) {
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
          if (!whichPtxt(core_schedule)) {
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

void execute_validation_schedule(std::map<string, double>& validation_regs, 
                     std::map<string, std::shared_ptr<std::mutex>>& reg_locks,
                     vector<queue<Node*>> schedule) {
  omp_set_num_threads(schedule.size());
  #pragma omp parallel for
  for (uint64_t i = 0; i < schedule.size(); i++) {
    auto &core_schedule = schedule[i];
    while(!core_schedule.empty()) {
      auto output_index = core_schedule.front()->get_output();
      auto inputs = core_schedule.front()->get_inputs();
      auto input_index1 = inputs[0];
      string input_index2 = "";
      if (inputs.size() > 1) {
        input_index2 = inputs[1];
        handle_input_mutex(reg_locks, input_index2);
      }
      handle_input_mutex(reg_locks, input_index1);
      switch(core_schedule.front()->get_op()) {
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

bool doubles_close_enough(double experimental, double expected) {
  if (expected == 0) {
    return false;
    // return std::abs(experimental) < 0.000000001;
  }
  return (std::abs(experimental - expected)/expected) < 0.005;
}

void validate_results(std::map<string, Ciphertext<DCRTPoly>>& enc_regs, 
                     std::map<string, double>& validation_regs,
                     PrivateKey<DCRTPoly>& private_key,
                     CryptoContext<DCRTPoly>& context) {
  for (auto &[key, ctxt] : enc_regs) {
    Plaintext tmp_ptxt;
    context->Decrypt(private_key, ctxt, &tmp_ptxt);
    auto decrypted_val = tmp_ptxt->GetRealPackedValue()[0];

    // if (!doubles_close_enough(decrypted_val, validation_regs[key])) {
      std::cout << key << ": FHE result: " << decrypted_val << ", expected: " << validation_regs[key] << std::endl;
    // }
  }
}

void gen_random_vals(std::map<string, Ciphertext<DCRTPoly>>& enc_regs, 
                     std::map<string, Plaintext>& ptxt_regs, 
                     std::map<string, double>& validation_regs, 
                     vector<queue<Node*>> schedule, PublicKey<DCRTPoly>& pub_key, 
                     CryptoContext<DCRTPoly>& context) {
  for (uint64_t i = 0; i < schedule.size(); i++) {
    while (!schedule[i].empty()) {
      for (uint64_t j = 0; j < schedule[i].front()->get_inputs().size(); j++) {
        bool is_ctxt = schedule[i].front()->get_inputs()[j].find('p') == std::string::npos;
        if (is_ctxt) {
          if (enc_regs.find(schedule[i].front()->get_inputs()[j]) == enc_regs.end()) {
            vector<double> tmp_vec; 
            tmp_vec.push_back(static_cast <double> (rand()) / static_cast <float> (RAND_MAX));
            auto tmp_ptxt = context->MakeCKKSPackedPlaintext(tmp_vec);
            auto tmp = context->Encrypt(pub_key, tmp_ptxt);
            enc_regs[schedule[i].front()->get_inputs()[j]] = tmp;
            validation_regs[schedule[i].front()->get_inputs()[j]] = tmp_vec[0];
          }
        }
        else {
          if (ptxt_regs.find(schedule[i].front()->get_inputs()[j]) == ptxt_regs.end()) {
            vector<double> tmp_vec;
            tmp_vec.push_back(static_cast <double> (rand()) / static_cast <float> (RAND_MAX));
            auto tmp_ptxt = context->MakeCKKSPackedPlaintext(tmp_vec);
            ptxt_regs[schedule[i].front()->get_inputs()[j]] = tmp_ptxt;
            validation_regs[schedule[i].front()->get_inputs()[j]] = tmp_vec[0];
          }
        }
      }
      schedule[i].pop();
    }
  }
  return;
}

void get_reg_locks(std::map<string, std::shared_ptr<std::mutex>> &reg_locks,
                    vector<queue<Node*>> schedule) {
  for (uint64_t i = 0; i < schedule.size(); i++) {
    auto &core_schedule = schedule[i];
    while(!core_schedule.empty()) {
      auto output_index = core_schedule.front()->get_output();
      reg_locks.emplace(output_index, new std::mutex);
      reg_locks[output_index]->lock();
      core_schedule.pop();
    }
  }
}

void lock_all_mutexes(std::map<string, std::shared_ptr<std::mutex>> &reg_locks) {
  for (auto &[key, mutex] : reg_locks) {
    mutex->lock();
  }
}

int main(int argc, char **argv) {
  // Set up crypto context
  CCParams<CryptoContextCKKSRNS> parameters;
  SecretKeyDist secretKeyDist = UNIFORM_TERNARY;
  parameters.SetSecretKeyDist(secretKeyDist);
  // parameters.SetSecurityLevel(HEStd_128_classic);
  parameters.SetSecurityLevel(HEStd_NotSet);
  parameters.SetRingDim(1 << 12);

#if NATIVEINT == 128
  ScalingTechnique rescaleTech = FIXEDAUTO;
  usint dcrtBits               = 78;
  usint firstMod               = 89;
#else
  ScalingTechnique rescaleTech = FLEXIBLEAUTO;
  usint dcrtBits               = 59;
  usint firstMod               = 60;
#endif

  parameters.SetScalingModSize(dcrtBits);
  parameters.SetScalingTechnique(rescaleTech);
  parameters.SetFirstModSize(firstMod);

  std::vector<uint32_t> levelBudget = {4, 4};
  uint32_t approxBootstrapDepth     = 8;

  uint32_t levelsUsedBeforeBootstrap = 10;
  usint depth =
      levelsUsedBeforeBootstrap + FHECKKSRNS::GetBootstrapDepth(approxBootstrapDepth, levelBudget, secretKeyDist);
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

  int num_workers = 1;
  string filename = "";
  if (argc != 3) {
    cout << "Usage: ./execution_engine [schedule_file] [num_threads]" << endl;
    exit(0);
  }
  else {
    try {
      num_workers = atoi(argv[2]);
    }
    catch(...) {
      cout << "Invalid thread number, using sequential mode." << endl;
      num_workers = 1;
    }
    filename = argv[1];
  }
  vector<queue<Node*>> schedule = parse_schedule(filename, num_workers);
  std::map<string, Ciphertext<DCRTPoly>> e_regs;
  std::map<string, Plaintext> p_regs;
  std::map<string, double> v_regs;
  srand(time(NULL));
  gen_random_vals(e_regs, p_regs, v_regs, schedule, keyPair.publicKey, cryptoContext);

  std::map<string, std::shared_ptr<std::mutex>> reg_locks;
  get_reg_locks(reg_locks, schedule);

  cout << "Executing in ciphertext..." << endl;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  execute_schedule(e_regs, p_regs, reg_locks, schedule, keyPair.publicKey, cryptoContext);
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  cout << "Done." << endl;
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
  cout << "Eval Time: " << time_span.count() << " seconds." << endl;

  lock_all_mutexes(reg_locks);
  cout << "Executing in plaintext..." << endl;
  execute_validation_schedule(v_regs, reg_locks, schedule);
  cout << "Done." << endl;
  cout << "Comparing ctxt results against ptxt results..." << endl;
  validate_results(e_regs, v_regs, keyPair.secretKey, cryptoContext);
  cout << "Done." << endl;
  return 0;
}