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

  if (options.mode == ExecMode::PLAINTEXT || options.verify_results)
  {
    execute_in_plaintext();
  }

  if (options.mode == ExecMode::PLAINTEXT)
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
    generate_inputs();
    generate_reg_locks();
    generate_dependence_locks();
  };
  utl::perform_func_and_print_execution_time(struct_funcs, "Generating inputs and mutexes");
}

void ExecutionEngine::generate_inputs()
{
  switch (options.input_mode)
  {
  case InputMode::CONSTANT:
    generate_constant_inputs();
    break;
  case InputMode::RANDOM:
    generate_random_inputs();
    break;
  case InputMode::FILE:
    generate_inputs_from_file();
    break;
  default:
    std::cout << "Invalid input mode." << std::endl;
    exit(1);
    break;
  }
}

void ExecutionEngine::generate_constant_inputs()
{
  for (const auto &input : sched_info.initial_inputs)
  {
    validation_regs[input.key] = options.inputs_value;
  }
}

void ExecutionEngine::generate_random_inputs()
{
  for (const auto &input : sched_info.initial_inputs)
  {
    validation_regs[input.key] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX / options.rand_thresh);
  }
}

void ExecutionEngine::generate_inputs_from_file()
{
  std::ifstream inputs_file(options.inputs_filename);

  auto line = utl::get_trimmed_line_from_file(inputs_file);
  while (!line.empty())
  {
    auto line_as_list = utl::split_string_by_character(line, ',');
    validation_regs[line_as_list[0]] = std::stod(line_as_list[1]);
    line = utl::get_trimmed_line_from_file(inputs_file);
  }

  if (validation_regs.size() != sched_info.initial_inputs.size())
  {
    std::cout << "Invalid inputs file. Number of inputs does not match schedule." << std::endl;
    exit(1);
  }

  for (const auto &input : sched_info.initial_inputs)
  {
    if (!validation_regs.count(input.key))
    {
      std::cout << "Invalid inputs file. Missing input with key " << input.key << "." << std::endl;
      exit(1);
    }
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
      handle_input_mutex(inputs[0].key);
      if (inputs.size() == 2)
      {
        handle_input_mutex(inputs[1].key);
      }
      map_lock.lock();
      auto input1 = validation_regs[inputs[0].key];
      double input2=0;//Mihailo put this value to 0 to solve issues of warning being treated as errors
      if (inputs.size() == 2)
      {
        input2 = validation_regs[inputs[1].key];
      }
      map_lock.unlock();
      double result;
      switch (operation->get_op_type())
      {
      case CP_MUL:
      case CC_MUL:
        result = input1 * input2;
        break;
      case CP_ADD:
      case CC_ADD:
        result = input1 + input2;
        break;
      case CP_SUB:
      case CC_SUB:
        result = input1 - input2;
        break;
      case PC_SUB:
        result = input2 - input1;
        break;
      case INV:
        result = -input1;
        break;
      case BOOT:
        result = input1;
        break;
      default:
        std::cout << "Invalid Instruction! Exiting..." << std::endl;
        exit(-1);
      }

      map_lock.lock();
      validation_regs[output_key] = result;
      map_lock.unlock();

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
    if (abs(value) > 0.000001 && abs(value) < 10000)
    {
      good_count++;
    }
    else
    {
      bad_count++;
    }
    std::cout << key << ": " << value << std::endl;
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
  for (int i = 0; i < (int) ctxt_regs.size(); i++)//Mihailo added casting to solve issue of warnings being treated as errors (comparison of integer expressions of different signedness)
  {
    auto it = ctxt_regs.begin();
    advance(it, i);
    auto &[_, ctxt] = *it;
    auto tmp = context->EvalBootstrap(ctxt);
    map_lock.lock();
    ctxt = tmp;
    /* The below code can be commented in for testing
    with ciphertexts at the noise threshold */
    // for (int i = 0; i < options.num_levels; i++)
    // {
    //   ctxt = context->EvalMult(ctxt, ctxt);
    // }
    map_lock.unlock();
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

ExecutionEngine::Ctxt ExecutionEngine::get_ctxt_input(const std::vector<EngineOpInput> &inputs)
{
  map_lock.lock();
  auto input_ctxt = ctxt_regs[inputs[0].key];
  map_lock.unlock();
  return input_ctxt;
}

std::pair<ExecutionEngine::Ctxt, ExecutionEngine::Ctxt> ExecutionEngine::get_ctxt_ctxt_inputs(const std::vector<EngineOpInput> &inputs)
{
  map_lock.lock();
  auto inputs_pair = std::pair<Ctxt, Ctxt>{ctxt_regs[inputs[0].key], ctxt_regs[inputs[1].key]};
  map_lock.unlock();
  return inputs_pair;
}

std::pair<ExecutionEngine::Ctxt, ExecutionEngine::Ptxt> ExecutionEngine::get_ctxt_ptxt_inputs(const std::vector<EngineOpInput> &inputs)
{
  map_lock.lock();
  auto inputs_pair = std::pair<Ctxt, Ptxt>{ctxt_regs[inputs[0].key], ptxt_regs[inputs[1].key]};
  map_lock.unlock();
  return inputs_pair;
}

int ExecutionEngine::execute_schedule()
{
  const bool ALAP_mode = (options.mode == ExecMode::ALAP);
  std::atomic<int> bootstrap_counter = 0;
#pragma omp parallel for
  for (size_t i = 0; i < sched_info.circuit.size(); i++)
  {
    for (const auto &operation : sched_info.circuit[i])
    {
      auto output_key = operation->get_output();
      auto inputs = operation->get_inputs();
      handle_input_mutex(inputs[0].key);
      bool has_2_inputs = (inputs.size() == 2);
      if (has_2_inputs)
      {
        handle_input_mutex(inputs[1].key);
      }
      Ctxt result;
      switch (operation->get_op_type())
      {
      case CP_MUL:
      {
        const auto [input1, input2] = get_ctxt_ptxt_inputs(inputs);
        result = context->EvalMult(input1, input2);
        context->ModReduceInPlace(result);
      }
      break;
      case CC_MUL:
      {
        const auto [input1, input2] = get_ctxt_ctxt_inputs(inputs);
        result = context->EvalMult(input1, input2);
        context->ModReduceInPlace(result);
        break;
      }
      case CP_ADD:
      {
        const auto [input1, input2] = get_ctxt_ptxt_inputs(inputs);
        result = context->EvalAdd(input1, ptxt_regs[inputs[1].key]);
        break;
      }
      case CC_ADD:
      {
        const auto [input1, input2] = get_ctxt_ctxt_inputs(inputs);
        result = context->EvalAdd(input1, input2);
        break;
      }
      case CP_SUB:
      {
        const auto [input1, input2] = get_ctxt_ptxt_inputs(inputs);
        result = context->EvalSub(input1, ptxt_regs[inputs[1].key]);
        break;
      }
      case PC_SUB:
      {
        const auto [input1, input2] = get_ctxt_ptxt_inputs(inputs);
        result = context->EvalSub(input1, ptxt_regs[inputs[1].key]);
        result = context->EvalNegate(result);
      }
      case CC_SUB:
      {
        const auto [input1, input2] = get_ctxt_ctxt_inputs(inputs);
        result = context->EvalSub(input1, input2);
        break;
      }
      case INV:
      {
        const auto input1 = get_ctxt_input(inputs);
        result = context->EvalNegate(input1);
        break;
      }
      case BOOT:
      {
        const auto input1 = get_ctxt_input(inputs);
        result = context->EvalBootstrap(input1);
        bootstrap_counter++;
        break;
      }
      default:
        std::cout << "Invalid Instruction! Exiting..." << std::endl;
        exit(-1);
      }
      if (ALAP_mode &&
          sched_info.bootstrap_candidates.count(output_key) &&
          result->GetLevel() >= (long unsigned int) level_to_bootstrap)//Mihailo added casting to solve issue of warnings being treated as errors (comparison of integer expressions of different signedness)
      {
        result = context->EvalBootstrap(result);
        bootstrap_counter++;
      }

      map_lock.lock();
      ctxt_regs[output_key] = result;
      map_lock.unlock();

      reg_locks[output_key]->unlock();

      // std::cout << output_key << " level: " << result->GetLevel() << std::endl;
      // if (result->GetLevel() == 0)
      // {
      //   std::cout << inputs[0].key << " level: " << ctxt_regs[inputs[0].key]->GetLevel() << std::endl;
      //   if (has_2_inputs)
      //   {
      //     std::cout << inputs[1].key << " level: " << ctxt_regs[inputs[1].key]->GetLevel() << std::endl;
      //   }
      // }

      update_dependence_info(inputs[0], output_key);
      if (has_2_inputs)
      {
        update_dependence_info(inputs[1], output_key);
      }
    }
  }
  return bootstrap_counter;
}

void ExecutionEngine::update_dependence_info(const EngineOpInput &input, const std::string &output_key)
{
  const auto &input_key = input.key;
  dependence_locks[input_key]->lock();
  map_lock.lock();
  sched_info.dependent_outputs[input_key].erase(output_key);
  if (sched_info.dependent_outputs[input_key].empty())
  {
    if (input.is_ctxt)
    {
      ctxt_regs.erase(input_key);
      // reg_locks.erase(input_key);
    }
    else
    {
      ptxt_regs.erase(input_key);
    }
  }
  map_lock.unlock();
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
  const double error_threshold = 0.5;

  for (const auto &[key, ctxt] : ctxt_regs)
  {
    Ptxt tmp_ptxt;
    context->Decrypt(key_pair.secretKey, ctxt, &tmp_ptxt);
    auto decrypted_val = tmp_ptxt->GetRealPackedValue()[0];

    auto percent_error = utl::get_percent_error(decrypted_val, validation_regs.at(key));
    if (percent_error > error_threshold)
    {
      std::cout << "WARNING!" << std::endl;
      std::cout << "WARNING!" << std::endl;
      std::cout << "The following result has an error greater than the threshold of " << error_threshold << "%." << std::endl;
    }
    std::cout << key << ": FHE result: " << decrypted_val << ", expected: " << validation_regs.at(key) << ", error: " << percent_error << "%" << std::endl;
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
  const int minimum_arguments = 2;

  if (argc < minimum_arguments)
  {
    std::cout << help_info << std::endl;
  }

  std::string sched_filename_no_ext = argv[1];
  options.sched_filename = sched_filename_no_ext + ".sched";

  std::string options_string = utl::make_options_string(argc, argv, minimum_arguments);

  auto num_levels_string = utl::get_arg(options_string, "-l", "--num-levels", help_info);
  if (!num_levels_string.empty())
  {
    options.num_levels = stoi(num_levels_string);
  }

  auto input_mode_pair_string = utl::get_arg(options_string, "-i", "--input_mode", help_info);

  if (input_mode_pair_string.empty())
  {
    options.input_mode_string = "CONSTANT";
    options.input_mode = InputMode::CONSTANT;
  }
  else
  {
    auto input_mode_pair_list = utl::split_string_by_character(input_mode_pair_string, ',');
    options.input_mode_string = input_mode_pair_list[0];
    auto value = input_mode_pair_list[1];
    if (options.input_mode_string == "CONSTANT")
    {
      options.input_mode = InputMode::CONSTANT;
      options.inputs_value = std::stod(value);
    }
    else if (options.input_mode_string == "RANDOM")
    {
      options.input_mode = InputMode::RANDOM;
      options.rand_thresh = std::stod(value);
    }
    else if (options.input_mode_string == "FILE")
    {
      options.input_mode = InputMode::FILE;
      options.inputs_filename = value;
    }
    else
    {
      throw std::invalid_argument(options.input_mode_string + "is not a valid input mode.");
    }
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
    options.mode = ExecMode::BOOSTER;
  }
  else if (options.mode_string == "BOOSTER")
  {
    options.mode = ExecMode::BOOSTER;
  }
  else if (options.mode_string == "ALAP")
  {
    options.mode = ExecMode::ALAP;
  }
  else if (options.mode_string == "PLAINTEXT")
  {
    options.mode = ExecMode::PLAINTEXT;
  }
  else
  {
    throw std::invalid_argument(options.mode_string + "is not a valid execution mode.");
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
    options.eval_time_filename = sched_filename_no_ext + "_eval_time_" + options.mode_string + "_" + output_suffix + ".txt";
  }

  if (utl::arg_exists(options_string, "-s", "--save-num-bootstraps"))
  {
    options.num_bootstraps_filename = sched_filename_no_ext + "_num_bootstraps_" + options.mode_string + ".txt";
  }
}

void ExecutionEngine::print_options() const
{
  std::cout << "FHE-Runner using the following options." << std::endl;
  std::cout << "num_levels: " << options.num_levels << std::endl;
  std::cout << "input_mode: " << options.input_mode_string << std::endl;
  switch (options.input_mode)
  {
  case InputMode::CONSTANT:
    std::cout << "inputs_value: " << options.inputs_value << std::endl;
    break;
  case InputMode::RANDOM:
    std::cout << "rand_thresh: " << options.rand_thresh << std::endl;
    break;
  case InputMode::FILE:
    std::cout << "inputs_filename: " << options.inputs_filename << std::endl;
    break;
  }
  std::cout << "mode: " << options.mode_string << std::endl;
  std::cout << "verify_results: " << (options.verify_results ? "yes" : "no") << std::endl;
  std::cout << "bootstrap_inputs: " << (options.bootstrap_inputs ? "yes" : "no") << std::endl;
  std::cout << "sched_filename: " << options.sched_filename << std::endl;
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
