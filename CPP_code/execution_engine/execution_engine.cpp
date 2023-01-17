#include "map"
#include <cstdlib>
#include "omp.h"
#include <ctime>
#include <ratio>
#include <chrono>

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

std::pair<Ciphertext<DCRTPoly>, Plaintext> get_ctxt_ptxt_args(const std::queue<Node *> &core_schedule,
                    const std::map<string, Ciphertext<DCRTPoly>>& enc_regs,
                    const std::map<string, Plaintext>& ptxt_regs, 
                    const CryptoContext<DCRTPoly>& context) {
  auto input0 = core_schedule.front()->get_inputs()[0];
  auto input1 = core_schedule.front()->get_inputs()[1];

  return whichPtxt(core_schedule) ? 
    std::pair(enc_regs.at(input0), ptxt_regs.at(input1)) :
    std::pair(enc_regs.at(input1), ptxt_regs.at(input0));
}

void execute_schedule(std::map<string, Ciphertext<DCRTPoly>>& enc_regs, 
                     std::map<string, Plaintext>& ptxt_regs, 
                     vector<queue<Node*>> schedule, PublicKey<DCRTPoly>& pub_key, 
                     CryptoContext<DCRTPoly>& context) {
  omp_set_num_threads(schedule.size());
  #pragma omp parallel for
  for (uint64_t i = 0; i < schedule.size(); i++) {
    auto &core_schedule = schedule[i];
    std::pair<Ciphertext<DCRTPoly>, Plaintext> args;
    while(!schedule[i].empty()) {
      auto output_index = schedule[i].front()->get_output();
      switch(schedule[i].front()->get_op()) {
        case CMUL:
          args = get_ctxt_ptxt_args(core_schedule, enc_regs, ptxt_regs, context);
          enc_regs[output_index] = context->EvalMult(args.first, args.second);
          context->ModReduceInPlace(enc_regs[output_index]);
          break;
        case EMUL:
          enc_regs[output_index] = 
            context->EvalMult(enc_regs[schedule[i].front()->get_inputs()[0]], 
              enc_regs[schedule[i].front()->get_inputs()[1]]); 
          context->ModReduceInPlace(enc_regs[output_index]);
          break;
        case CADD:
          args = get_ctxt_ptxt_args(core_schedule, enc_regs, ptxt_regs, context);
          enc_regs[output_index] = context->EvalAdd(args.first, args.second);
          break;
        case EADD:
          enc_regs[output_index] = 
            context->EvalAdd(enc_regs[schedule[i].front()->get_inputs()[0]], 
              enc_regs[schedule[i].front()->get_inputs()[1]]); 
          break;
        case CSUB:
          args = get_ctxt_ptxt_args(core_schedule, enc_regs, ptxt_regs, context);
          enc_regs[output_index] = context->EvalSub(args.first, args.second);
          break;
        case ESUB:
          enc_regs[output_index] = 
            context->EvalSub(enc_regs[schedule[i].front()->get_inputs()[0]], 
              enc_regs[schedule[i].front()->get_inputs()[1]]); 
          break;
        case EINV:
          enc_regs[output_index] = 
            context->EvalNegate(enc_regs[schedule[i].front()->get_inputs()[0]]);
          break;
        case BOOT:
          enc_regs[output_index] = 
            context->EvalBootstrap(enc_regs[schedule[i].front()->get_inputs()[0]]);
          break;
        default:
          cout << "Invalid Instruction! Exiting..." << endl;
          exit(-1);
      }
      schedule[i].pop();
    }
  }
  return;
}

void gen_random_vals(std::map<string, Ciphertext<DCRTPoly>>& enc_regs, 
                     std::map<string, Plaintext>& ptxt_regs, 
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
          }
        }
        else {
          if (ptxt_regs.find(schedule[i].front()->get_inputs()[j]) == ptxt_regs.end()) {
            vector<double> tmp_vec;
            tmp_vec.push_back(static_cast <double> (rand()) / static_cast <float> (RAND_MAX));
            auto tmp_ptxt = context->MakeCKKSPackedPlaintext(tmp_vec);
            ptxt_regs[schedule[i].front()->get_inputs()[j]] = tmp_ptxt;
          }
        }
      }
      schedule[i].pop();
    }
  }
  return;
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
  gen_random_vals(e_regs, p_regs, schedule, keyPair.publicKey, cryptoContext);

  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  execute_schedule(e_regs, p_regs, schedule, keyPair.publicKey, cryptoContext);
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
  cout << "Eval Time: " << time_span.count() << " seconds." << endl;
  return 0;
}