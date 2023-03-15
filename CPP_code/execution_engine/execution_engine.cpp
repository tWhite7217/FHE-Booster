#include "execution_engine.hpp"

ExecutionEngine::ExecutionEngine(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << help_info << std::endl;
    exit(0);
  }
  else
  {
    try
    {
      parse_args(argc, argv);
      print_options();
    }
    catch (...)
    {
      std::cout << help_info << std::endl;
      exit(-1);
    }
  }
}

void ExecutionEngine::execute()
{
  parse_schedule();

  generate_data_structures();

  if (options.mode == PLAINTEXT || options.verify_results)
  {
    execute_in_plaintext();
  }

  if (options.mode == PLAINTEXT)
  {
    print_test_info();
  }
  else
  {
    setup_crypto_context();

    prepare_inputs();

    execute_in_ciphertext();

    if (options.verify_results)
    {
      verify_results();
    }

    write_execution_info_to_files();
  }
}

void ExecutionEngine::parse_schedule()
{
  std::function<void()> parser_func = [this]()
  {
    ScheduleParser parser;
    sched_info = parser.parse(options);
  };
  utl::perform_func_and_print_execution_time(parser_func, "Parsing schedule");

  omp_set_num_threads(sched_info.circuit.size());
}

void ExecutionEngine::generate_data_structures()
{
  std::function<void()> struct_funcs = [this]()
  {
    generate_random_inputs();
    generate_reg_locks();
    generate_dependence_locks();
  };
  utl::perform_func_and_print_execution_time(struct_funcs, "Generating random inputs and mutexes");
}

void ExecutionEngine::generate_random_inputs()
{
  for (const auto &input : sched_info.initial_inputs)
  {
    // validation_regs[key] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX / options.rand_thresh);
    validation_regs[input.key] = options.rand_thresh;
  }
}

void ExecutionEngine::generate_reg_locks()
{
  reg_locks.clear();
  for (uint64_t i = 0; i < sched_info.circuit.size(); i++)
  {
    for (const auto &operation : sched_info.circuit[i])
    {
      auto output_key = operation->get_output();
      reg_locks.emplace(output_key, new std::mutex);
      reg_locks[output_key]->lock();
    }
  }
}

void ExecutionEngine::generate_dependence_locks()
{
  dependence_locks.clear();
  for (const auto &[input_key, _] : sched_info.dependent_outputs)
  {
    dependence_locks.emplace(input_key, new std::mutex);
  }
}

void ExecutionEngine::execute_in_plaintext()
{
  std::function<void()> ptxt_func = [this]()
  {
    execute_validation_schedule();
    lock_all_mutexes();
  };
  utl::perform_func_and_print_execution_time(ptxt_func, "Executing in plaintext");
}

void ExecutionEngine::execute_validation_schedule()
{
#pragma omp parallel for
  for (size_t i = 0; i < sched_info.circuit.size(); i++)
  {
    for (const auto &operation : sched_info.circuit[i])
    {
      auto output_key = operation->get_output();
      auto inputs = operation->get_inputs();
      auto input_key1 = inputs[0].key;
      handle_input_mutex(input_key1);
      std::string input_key2;
      if (inputs.size() == 2)
      {
        input_key2 = inputs[1].key;
        handle_input_mutex(input_key2);
      }
      switch (operation->get_op_type())
      {
      case CP_MUL:
      case CC_MUL:
        validation_regs[output_key] =
            validation_regs[input_key1] * validation_regs[input_key2];
        break;
      case CP_ADD:
      case CC_ADD:
        validation_regs[output_key] =
            validation_regs[input_key1] + validation_regs[input_key2];
        break;
      case CP_SUB:
      case CC_SUB:
        validation_regs[output_key] =
            validation_regs[input_key1] - validation_regs[input_key2];
        break;
      case PC_SUB:
        validation_regs[output_key] =
            validation_regs[input_key2] - validation_regs[input_key1];
        break;
      case INV:
        validation_regs[output_key] = -validation_regs[input_key1];
        break;
      case BOOT:
        validation_regs[output_key] = validation_regs[input_key1];
        break;
      default:
        std::cout << "Invalid Instruction! Exiting..." << std::endl;
        exit(-1);
      }
      reg_locks[output_key]->unlock();
    }
  }
  return;
}

void ExecutionEngine::handle_input_mutex(const std::string &input_key)
{
  if (reg_locks.count(input_key))
  {
    reg_locks[input_key]->lock();
    reg_locks[input_key]->unlock();
  }
}

void ExecutionEngine::lock_all_mutexes()
{
  for (auto &[key, mutex] : reg_locks)
  {
    mutex->lock();
  }
}

void ExecutionEngine::print_test_info() const
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

void ExecutionEngine::setup_crypto_context()
{
  std::function<void()> crypto_func = [this]()
  {
    lbcrypto::CCParams<lbcrypto::CryptoContextCKKSRNS> parameters;
    SecretKeyDist secretKeyDist = UNIFORM_TERNARY;
    parameters.SetSecretKeyDist(secretKeyDist);
    // parameters.SetSecurityLevel(HEStd_128_classic);
    parameters.SetSecurityLevel(lbcrypto::HEStd_NotSet);
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

    auto ctxt_level_after_bootstrap = lbcrypto::FHECKKSRNS::GetBootstrapDepth(approxBootstrapDepth, levelBudget, secretKeyDist);
    // std::cout << "ctxt_level_after_bootstrap: " << ctxt_level_after_bootstrap << std::endl;
    level_to_bootstrap = ctxt_level_after_bootstrap + options.num_levels;
    unsigned int depth = level_to_bootstrap + 2;
    parameters.SetMultiplicativeDepth(depth);

    context = GenCryptoContext(parameters);

    context->Enable(PKE);
    context->Enable(KEYSWITCH);
    context->Enable(LEVELEDSHE);
    context->Enable(ADVANCEDSHE);
    context->Enable(FHE);

    unsigned int ringDim = context->GetRingDimension();
    unsigned int numSlots = ringDim / 2;

    context->EvalBootstrapSetup(levelBudget);

    key_pair = context->KeyGen();
    context->EvalMultKeyGen(key_pair.secretKey);
    context->EvalBootstrapKeyGen(key_pair.secretKey, numSlots);
  };
  utl::perform_func_and_print_execution_time(crypto_func, "Setting up crypto context");
}

void ExecutionEngine::prepare_inputs()
{
  std::function<void()> encrypt_func = [this]()
  { encrypt_inputs(); };
  utl::perform_func_and_print_execution_time(encrypt_func, "Encrypting inputs");

  std::string num_inputs = std::to_string(sched_info.initial_inputs.size());

  if (options.bootstrap_inputs)
  {
    std::function<void()> boot_func = [this]()
    { bootstrap_initial_inputs(); };
    utl::perform_func_and_print_execution_time(boot_func, "Bootstrapping " + num_inputs + " inputs");
  }
}

void ExecutionEngine::encrypt_inputs()
{
  for (const auto &input : sched_info.initial_inputs)
  {
    auto value = validation_regs[input.key];
    std::vector<double> tmp_vec;
    tmp_vec.push_back(value);
    auto tmp_ptxt = context->MakeCKKSPackedPlaintext(tmp_vec);

    if (input.is_ctxt)
    {
      auto tmp = context->Encrypt(key_pair.publicKey, tmp_ptxt);
      ctxt_regs[input.key] = tmp;
    }
    else
    {
      ptxt_regs[input.key] = tmp_ptxt;
    }
  }
}

void ExecutionEngine::bootstrap_initial_inputs()
{
#pragma omp parallel for
  for (int i = 0; i < ctxt_regs.size(); i++)
  {
    auto it = ctxt_regs.begin();
    advance(it, i);
    auto &[_, ctxt] = *it;
    ctxt = context->EvalBootstrap(ctxt);
    /* The below code can be commented in for testing
    with ciphertexts at the noise threshold */
    // for (int i = 0; i < options.num_levels; i++)
    // {
    //   ctxt = context->EvalMult(ctxt, ctxt);
    // }
  }
}

void ExecutionEngine::execute_in_ciphertext()
{
  std::function<int()> exec_func = [this]()
  { return execute_schedule(); };
  num_bootstraps = utl::perform_func_and_print_execution_time(
      exec_func, "Executing in ciphertext", &execution_time);
  std::cout << "Number of bootstrap operations: " << num_bootstraps << "." << std::endl;
}

int ExecutionEngine::execute_schedule()
{
  const bool ALAP_mode = (options.mode == ALAP);
  std::atomic<int> bootstrap_counter = 0;
#pragma omp parallel for
  for (size_t i = 0; i < sched_info.circuit.size(); i++)
  {
    for (const auto &operation : sched_info.circuit[i])
    {
      auto output_key = operation->get_output();
      // auto input_indices = get_input_indices(operation);
      // auto input_key1 = input_indices.first;
      // auto input_key2 = input_indices.second;
      auto inputs = operation->get_inputs();
      handle_input_mutex(inputs[0].key);
      bool has_2_inputs = (inputs.size() == 2);
      if (has_2_inputs)
      {
        handle_input_mutex(inputs[1].key);
      }
      switch (operation->get_op_type())
      {
      case CP_MUL:
        ctxt_regs[output_key] =
            context->EvalMult(ctxt_regs[inputs[0].key], ptxt_regs[inputs[1].key]);
        context->ModReduceInPlace(ctxt_regs[output_key]);
        break;
      case CC_MUL:
        ctxt_regs[output_key] =
            context->EvalMult(ctxt_regs[inputs[0].key], ctxt_regs[inputs[1].key]);
        context->ModReduceInPlace(ctxt_regs[output_key]);
        break;
      case CP_ADD:
        ctxt_regs[output_key] =
            context->EvalAdd(ctxt_regs[inputs[0].key], ptxt_regs[inputs[1].key]);
        break;
      case CC_ADD:
        ctxt_regs[output_key] =
            context->EvalAdd(ctxt_regs[inputs[0].key], ctxt_regs[inputs[1].key]);
        break;
      case CP_SUB:
        ctxt_regs[output_key] =
            context->EvalSub(ctxt_regs[inputs[0].key], ptxt_regs[inputs[1].key]);
        break;
      case PC_SUB:
        ctxt_regs[output_key] =
            context->EvalSub(ctxt_regs[inputs[0].key], ptxt_regs[inputs[1].key]);
        ctxt_regs[output_key] = context->EvalNegate(ctxt_regs[output_key]);
      case CC_SUB:
        ctxt_regs[output_key] =
            context->EvalSub(ctxt_regs[inputs[0].key], ctxt_regs[inputs[1].key]);
        break;
      case INV:
        ctxt_regs[output_key] = context->EvalNegate(ctxt_regs[inputs[0].key]);
        break;
      case BOOT:
        ctxt_regs[output_key] = context->EvalBootstrap(ctxt_regs[inputs[0].key]);
        bootstrap_counter++;
        break;
      default:
        std::cout << "Invalid Instruction! Exiting..." << std::endl;
        exit(-1);
      }
      if (ALAP_mode &&
          sched_info.bootstrap_candidates.count(output_key) &&
          ctxt_regs[output_key]->GetLevel() >= level_to_bootstrap)
      {
        ctxt_regs[output_key] = context->EvalBootstrap(ctxt_regs[output_key]);
        bootstrap_counter++;
      }

      reg_locks[output_key]->unlock();

      update_dependence_info(inputs[0], output_key);
      if (has_2_inputs)
      {
        update_dependence_info(inputs[1], output_key);
      }

      // std::cout << output_key << " level: " << ctxt_regs[output_key]->GetLevel() << std::endl;
    }
  }
  return bootstrap_counter;
}

void ExecutionEngine::update_dependence_info(const EngineOpInput &input, const std::string &output_key)
{
  const auto &input_key = input.key;
  dependence_locks[input_key]->lock();
  sched_info.dependent_outputs[input_key].erase(output_key);
  if (sched_info.dependent_outputs[input_key].empty())
  {
    if (input.is_ctxt)
    {
      ctxt_regs.erase(input_key);
      reg_locks.erase(input_key);
    }
    else
    {
      ptxt_regs.erase(input_key);
    }
  }
  dependence_locks[input_key]->unlock();
}

void ExecutionEngine::verify_results()
{
  std::function<void()> validate_func = [this]()
  { validate_results(); };
  utl::perform_func_and_print_execution_time(
      validate_func, "Comparing ctxt results against ptxt results");
}

void ExecutionEngine::validate_results() const
{
  for (const auto &[key, ctxt] : ctxt_regs)
  {
    Ptxt tmp_ptxt;
    context->Decrypt(key_pair.secretKey, ctxt, &tmp_ptxt);
    auto decrypted_val = tmp_ptxt->GetRealPackedValue()[0];

    auto percent_error = utl::get_percent_error(decrypted_val, validation_regs.at(key));
    if (percent_error > 0.5)
    {
      std::cout << key << ": FHE result: " << decrypted_val << ", expected: " << validation_regs.at(key) << ", error: " << percent_error << "%" << std::endl;
    }
  }
}

void ExecutionEngine::write_execution_info_to_files()
{
  if (!options.eval_time_filename.empty())
  {
    std::ofstream time_file(options.eval_time_filename);
    time_file << execution_time << std::endl;
    time_file.close();
  }
  if (!options.num_bootstraps_filename.empty())
  {
    std::ofstream segments_file(options.num_bootstraps_filename);
    segments_file << num_bootstraps << std::endl;
    segments_file.close();
  }
}

void ExecutionEngine::print_schedule() const
{
  int i = 0;
  for (const auto &core_schedule : sched_info.circuit)
  {
    std::cout << "=== Nodes for Worker " << i << " ===" << std::endl;
    for (const auto &operation : core_schedule)
    {
      operation->print();
    }
    i++;
  }
}

void ExecutionEngine::parse_args(int argc, char **argv)
{
  std::string sched_filename = argv[1];
  options.input_filename = sched_filename + ".sched";

  std::string options_string;
  for (auto i = 2; i < argc; i++)
  {
    options_string += std::string(argv[i]) + " ";
  }

  auto num_levels_string = utl::get_arg(options_string, "-l", "--num-levels", help_info);
  if (!num_levels_string.empty())
  {
    options.num_levels = stoi(num_levels_string);
  }

  auto rand_thresh_string = utl::get_arg(options_string, "-r", "--rand-thresh", help_info);
  if (!rand_thresh_string.empty())
  {
    options.rand_thresh = stod(rand_thresh_string);
  }

  options.mode_string = utl::get_arg(options_string, "-m", "--mode", help_info);
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

  if (utl::arg_exists(options_string, "-v", "--verify"))
  {
    options.verify_results = true;
  }

  if (utl::arg_exists(options_string, "-b", "--bootstrap-inputs"))
  {
    options.bootstrap_inputs = true;
  }

  auto output_suffix = utl::get_arg(options_string, "-o", "--output-suffix", help_info);
  if (!output_suffix.empty())
  {
    options.eval_time_filename = sched_filename + "_eval_time_" + options.mode_string + "_" + output_suffix + ".txt";
  }

  if (utl::arg_exists(options_string, "-s", "--save-num-bootstraps"))
  {
    options.num_bootstraps_filename = sched_filename + "_num_bootstraps_" + options.mode_string + ".txt";
  }
}

void ExecutionEngine::print_options() const
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
  srand(time(NULL));

  auto engine = ExecutionEngine(argc, argv);
  engine.execute();

  return 0;
}