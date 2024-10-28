#define STRINGIFY_BACKEND(A) #A
#define STRINGIFY(A) STRINGIFY_BACKEND(A)

#include "include.h"
#include "timing.h"
#include "data.h"
#include <experimental/xrt_queue.h>

#include <random>

int main(int argc, char* argv[]) {

  if(argc != 5)
    throw std::runtime_error(std::string("Not correct amount of parameters provided. Usage: ") + argv[0] + " </path/to/a.xclbin> <number of alignments> <number of plf calls> <parallel instances used>\n");

  acap_info acap(argv[1]);
  testbench_info tb;
  try {
    tb.alignment_sites = std::stoul(argv[2]);
  } catch (const std::invalid_argument& ia) {
    std::cerr << "Invalid number for alignment_sites: " << ia.what() << std::endl;
  } catch (const std::out_of_range& oor) {
    std::cerr << "Argument(alignment sites) out of range" << std::endl;
  }
  try {
    tb.plf_calls = std::stoul(argv[3]);
  } catch (const std::invalid_argument& ia) {
    std::cerr << "Invalid number for plf_calls: " << ia.what() << std::endl;
  } catch (const std::out_of_range& oor) {
    std::cerr << "Argument(plf calls) out of range" << std::endl;
  }
  try {
    tb.parallel_instances = std::stoul(argv[4]);
  } catch (const std::invalid_argument& ia) {
    std::cerr << "Invalid number for parallel_instances: " << ia.what() << std::endl;
  } catch (const std::out_of_range& oor) {
    std::cerr << "Argument(instances used) out of range" << std::endl;
  }
  tb.input_layout = acap.classifyLayoutType(acap.get_pl_name());

  tb.aie_type = acap.classifyAieType(acap.get_pl_name());

  tb.window_size = acap.classifyWindowSize(acap.get_aie_name());

  std::cout << std::left << std::endl;
  std::cout << "====================================================================================" << std::endl;
  std::cout << "| test name:              | " << std::setw(54) << acap.get_app_name() << " |" << std::endl;
  std::cout << "| PL name:                | " << std::setw(54) << acap.get_pl_name() << " |" << std::endl;
  std::cout << "| AIE name:               | " << std::setw(54) << acap.get_aie_name() << " |" << std::endl;
  std::cout << "====================================================================================" << std::endl;
  std::cout << "| alignment sites:        | " << std::setw(54) << tb.alignment_sites << " |" << std::endl;
  std::cout << "| plf calls:              | " << std::setw(54) << tb.plf_calls << " |" << std::endl;
  std::cout << "| parallel plfs:          | " << std::setw(54) << tb.parallel_instances << " |" << std::endl;
  std::cout << "------------------------------------------------------------------------------------" << std::endl;
  std::cout << "| AIE window size:        | " << std::setw(54) << tb.window_size << " |" << std::endl;
  std::cout << std::right;
  std::cout << "====================================================================================" << std::endl;
  std::cout << "|                         |       alignments |         elements |     size (bytes) |" << std::endl;
  std::cout << "|- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - |" << std::endl;
  std::cout << "| instance left:          | " << std::setw(16) << tb.alignments_per_instance() << " | ";
  std::cout <<                                   std::setw(16) << tb.instance_elements_left()  << " | ";
  std::cout <<                                   std::setw(16) << tb.instance_size_left()      << " |" << std::endl;
  std::cout << "| instance right:         | " << std::setw(16) << tb.alignments_per_instance() << " | ";
  std::cout <<                                   std::setw(16) << tb.instance_elements_right() << " | ";
  std::cout <<                                   std::setw(16) << tb.instance_size_right()     << " |" << std::endl;
  std::cout << "| instance out:           | " << std::setw(16) << tb.alignments_per_instance() << " | ";
  std::cout <<                                   std::setw(16) << tb.instance_elements_out()   << " | ";
  std::cout <<                                   std::setw(16) << tb.instance_size_out()       << " |" << std::endl;
  std::cout << "------------------------------------------------------------------------------------" << std::endl;
  std::cout << "| buffer left:            | " << std::setw(16) << tb.alignment_sites         << " | ";
  std::cout <<                                   std::setw(16) << tb.buffer_elements_left()  << " | ";
  std::cout <<                                   std::setw(16) << tb.buffer_size_left()      << " |" << std::endl;
  std::cout << "| buffer right:           | " << std::setw(16) << tb.alignment_sites         << " | ";
  std::cout <<                                   std::setw(16) << tb.buffer_elements_right() << " | ";
  std::cout <<                                   std::setw(16) << tb.buffer_size_right()     << " |" << std::endl;
  std::cout << "| buffer out:             | " << std::setw(16) << tb.alignment_sites         << " | ";
  std::cout <<                                   std::setw(16) << tb.buffer_elements_out()   << " | ";
  std::cout <<                                   std::setw(16) << tb.buffer_size_out()       << " |" << std::endl;
  std::cout << "------------------------------------------------------------------------------------" << std::endl;
  std::cout << "| total (" << std::setw(3) << tb.plf_calls << " plf calls):  | " << std::setw(16) << tb.alignment_sites * tb.plf_calls << " | ";
  std::cout <<                                   std::setw(16) << tb.data_elements()                << " | ";
  std::cout <<                                   std::setw(16) << tb.data_size()                    << " |" << std::endl;
  std::cout << "====================================================================================" << std::endl;
  std::cout << "| RAM usage (host):       | " << std::setw(14) << tb.host_mem_usage()/1000000000.0 << " GB of 256 GB (";
  std::cout << std::setw(12) << tb.host_mem_usage()/2560000000.0 << " % )" << std::setw(11) << " |" << std::endl;
  std::cout << "| RAM usage (Versal):     | " << std::setw(14) << tb.acap_mem_usage()/1000000000.0 << " GB of  12 GB (";
  std::cout << std::setw(12) << tb.acap_mem_usage()/120000000.0 << " % )" << std::setw(11) << " |" << std::endl;
  std::cout << "====================================================================================" << std::endl;
  std::cout << std::left;
  std::cout << "| device name:            | " << std::setw(54) << acap.get_device()->get_info<xrt::info::device::name>() << " |" << std::endl;
  std::cout << "| device bdf:             | " << std::setw(54) << acap.get_device()->get_info<xrt::info::device::bdf>() << " |" << std::endl;
  std::cout << "------------------------------------------------------------------------------------" << std::endl;
  std::cout << "| xclbin host_type:       | " << std::setw(54) << acap.get_target() << " |" << std::endl;
  std::cout << "| xclbin kernels:         | ";
  for (auto kernel : acap.get_kernels()) {
    std::cout << kernel.get_name() << " ";
  }
  std::cout << std::endl;
  //std::cout << "| xclbin uuid:         | " << xclbin.get_uuid().to_string() << std::endl;
  std::cout << "====================================================================================" << std::endl;
  std::cout << std::right << std::endl;



  //Init//////////////////////////////////////////////////////////////////////////////////////////////////

  // Create HLS kernel handles
  xrt::kernel* mm2sleft_kernels = new xrt::kernel[tb.parallel_instances];
  xrt::kernel* mm2sright_kernels = new xrt::kernel[tb.parallel_instances];
  xrt::kernel* s2mm_kernels = new xrt::kernel[tb.parallel_instances];
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    mm2sleft_kernels[i] = xrt::kernel(*acap.get_device(), *acap.get_uuid(), kernel_name("mm2sleft", i));
    mm2sright_kernels[i] = xrt::kernel(*acap.get_device(), *acap.get_uuid(), kernel_name("mm2sright", i));
    s2mm_kernels[i] = xrt::kernel(*acap.get_device(), *acap.get_uuid(), kernel_name("s2mm", i));
    printf("mm2sleft[%i] mem group: %i, ", i, mm2sleft_kernels[i].group_id(0));
    printf("mm2sright[%i] mem group: %i, ", i, mm2sright_kernels[i].group_id(0));
    printf("s2mm[%i] mem group: %i\n", i, s2mm_kernels[i].group_id(0));
    printf("s2mm[%i] mem group: %i\n", i, s2mm_kernels[i].group_id(1));
  }
  printf("\nconnected to datamover kernels\n");

  // Create Host <-> ACAP Memory buffers
  xrt::bo* in_left_plf = new xrt::bo[tb.parallel_instances];
  xrt::bo* in_right_plf = new xrt::bo[tb.parallel_instances];
  xrt::bo* out_plf = new xrt::bo[tb.parallel_instances];
  xrt::bo* out_scaler = new xrt::bo[tb.parallel_instances];
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    in_left_plf[i] = xrt::bo(*acap.get_device(), tb.instance_size_left(), xrt::bo::flags::device_only, mm2sleft_kernels[i].group_id(0));
    in_right_plf[i] = xrt::bo(*acap.get_device(), tb.instance_size_right(), xrt::bo::flags::device_only, mm2sright_kernels[i].group_id(0));
    out_plf[i] = xrt::bo(*acap.get_device(), tb.instance_size_out(), xrt::bo::flags::device_only, s2mm_kernels[i].group_id(0));
    out_scaler[i] = xrt::bo(*acap.get_device(), sizeof(char)*tb.alignments_per_instance(), xrt::bo::flags::device_only, s2mm_kernels[i].group_id(1));
    // TODO: properly account for alignment padding for the loops of both streams and windows
  }

  // Create the HLS kernel run handles and set the parameters
  xrt::run* mm2sleft_run = new xrt::run[tb.parallel_instances];
  xrt::run* mm2sright_run = new xrt::run[tb.parallel_instances];
  xrt::run* s2mm_run = new xrt::run[tb.parallel_instances];
  std::cout << "alignments per instance: " << tb.alignments_per_instance() << ", padding: " << tb.alignments_padding() << std::endl;
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    std::cout << "last instance[" << i << "]: " << ((i == (tb.parallel_instances-1)) && (i != 0)) << ", alignments: " << (tb.alignments_per_instance() - ((i == tb.parallel_instances-1) * tb.alignments_padding())) << std::endl;
    mm2sleft_run[i] = xrt::run(mm2sleft_kernels[i]);
    mm2sleft_run[i].set_arg(0, in_left_plf[i]);
    mm2sleft_run[i].set_arg(1, tb.alignments_per_instance(i));
    mm2sleft_run[i].set_arg(2, tb.window_size);

    mm2sright_run[i] = xrt::run(mm2sright_kernels[i]);
    mm2sright_run[i].set_arg(0, in_right_plf[i]);
    mm2sright_run[i].set_arg(1, tb.alignments_per_instance(i));
    mm2sright_run[i].set_arg(2, tb.window_size);

    s2mm_run[i] = xrt::run(s2mm_kernels[i]);
    s2mm_run[i].set_arg(0, out_plf[i]);
    s2mm_run[i].set_arg(1, out_scaler[i]);
    s2mm_run[i].set_arg(2, tb.alignments_per_instance(i));
    s2mm_run[i].set_arg(3, tb.window_size);
  }

  // In sw_emu the aie graph needs to be started manually
  if (acap.get_target() == "sw_emu") {
    std::cout << "enable graph" << std::endl;
    xrt::graph aie_graph(*acap.get_device(), *acap.get_uuid(), "mygraph");
    aie_graph.run( tb.plf_calls * ((tb.alignment_sites+63)>>6) );
  }

  //Check before run///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(NO_PRERUN_CHECK) || NO_PRERUN_CHECK == 0
  if(!prerun_check()){
    exit(0);
  }
#endif


  //Load data/////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "Initialize test data ... " << std::endl;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0f, 1.0f);

  float ev[16];
  float branchleft[64];
  float branchright[64];
  float* alignmentsleft = new float[tb.elements_per_plf()];
  float* alignmentsright = new float[tb.elements_per_plf()];

  // load ev
  for (unsigned long int j = 0; j < 16; j++) {
    ev[j] = dis(gen);
  }
  // load branch left and branch right
  for (unsigned long int j = 0; j < 64; j++) {
    branchleft[j] = dis(gen);
    branchright[j] = dis(gen);
  }
  // load alignment data
  for (unsigned long int j = 0; j < tb.elements_per_plf(); j++) {
    int exp = (j%64 < 16);
    float scale = std::pow(1.0e-12,exp);
    alignmentsleft[j] = dis(gen) * scale;
    alignmentsright[j] = dis(gen);
  }

  int* wgt = new int[tb.alignment_sites];
  for (unsigned long int i = 0; i < tb.alignment_sites; i++) {
    wgt[i] = 1;
  }

  int* scalerIncrement_versal = new int[tb.plf_calls];
  char** scalerVector_versal = new char*[tb.plf_calls];
  float** versalResult = new float*[tb.plf_calls];
  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    versalResult[i] = new float[tb.elements_per_plf()];
    scalerVector_versal[i] = new char[tb.alignment_sites];
  }

  std::cout << "Prepare data for transfer ... " << std::endl;

  float** dataLeftInput = new float*[tb.parallel_instances];
  float** dataRightInput = new float*[tb.parallel_instances];

  for (unsigned long int k = 0; k < tb.parallel_instances; k++) {
    dataLeftInput[k] = new float[tb.instance_elements_left()];
    dataRightInput[k] = new float[tb.instance_elements_right()];

#if !defined(NO_INTERMEDIATE_RESULTS) || NO_INTERMEDIATE_RESULTS == 0
    unsigned int alignment_offset = k*tb.alignmentelements_per_instance(0);

    std::copy(ev,                ev+16,                                          dataLeftInput[k]      );
    std::copy(branchleft,        branchleft+64,                                  dataLeftInput[k] + 16 );
    std::copy(&alignmentsleft[alignment_offset], &alignmentsleft[alignment_offset + tb.alignmentelements_per_instance(k)], dataLeftInput[k] + 80 );
    if (tb.input_layout == COMBINED) {
      std::copy(ev,                 ev+16,                                           dataRightInput[k]      );
      std::copy(branchright,        branchright+64,                                  dataRightInput[k] + 16 );
      std::copy(&alignmentsright[alignment_offset], &alignmentsright[alignment_offset + tb.alignmentelements_per_instance(k)], dataRightInput[k] + 80 );
    } else {
      std::copy(branchright,        branchright+64,                                  dataRightInput[k]      );
      std::copy(&alignmentsright[alignment_offset], &alignmentsright[alignment_offset + tb.alignmentelements_per_instance(k)], dataRightInput[k] + 64 );
    }
#endif
  }


  //Run///////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "Setup data gathering structures ... " << std::endl << std::endl;
  xrt::queue main_queue[tb.parallel_instances];
  xrt::queue right_queue[tb.parallel_instances];
#if !defined(NO_INTERMEDIATE_RESULTS) || NO_INTERMEDIATE_RESULTS == 0
  xrt::queue::event right_in_finished[tb.parallel_instances];
  xrt::queue::event right_out_finished[tb.parallel_instances];
#else
  xrt::queue output_queue[tb.parallel_instances];
  xrt::queue::event right_done[tb.parallel_instances];
  xrt::queue::event output_done[tb.parallel_instances];
#endif
  xrt::queue::event execution_finished[tb.parallel_instances];
  xrt::queue::event instance_done[tb.parallel_instances];


#if !defined(NO_INTERMEDIATE_RESULTS) || NO_INTERMEDIATE_RESULTS == 0
  timing_data* execution_ms = new timing_data[tb.parallel_instances];
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    execution_ms[i].init(tb.plf_calls);
  }
#else
  timing_data execution_ms;
  execution_ms.init(tb.plf_calls);
#endif

  auto xrt_profile = xrt::profile::user_range("roundtrip_exec_time", "The execution time of the full test");
  Timer t;

  char* scaler;
  float* dataOutput;

  std::cout << "Start PLF calculation on accelerator ... " << std::endl << std::endl;
  //t.reset();

  xrt_profile.start("roundtrip_exec_time");
  for (long int i = 0; i < tb.plf_calls; i++) {

#if !defined(NO_INTERMEDIATE_RESULTS) || NO_INTERMEDIATE_RESULTS == 0
    // Execute PLF calculation parallelized on Versal
    for (unsigned int k = 0; k < tb.parallel_instances; k++) {

      // set pointers for lamda functions
      scaler = scalerVector_versal[i] + k*tb.alignments_per_instance(0);
      dataOutput = versalResult[i] + (k*tb.alignmentelements_per_instance(0));

      // time: begin
      main_queue[k].enqueue([&execution_ms, k, i, t] {execution_ms[k].begin[i] = t.elapsed();});

      // write data to Versal
      main_queue[k].enqueue([&in_left_plf, &dataLeftInput, &tb, k] {in_left_plf[k].write(dataLeftInput[k], tb.instance_active_elements_left(k)*sizeof(float), 0); });
      right_in_finished[k] = right_queue[k].enqueue([&in_right_plf, &dataRightInput, &tb, k] {in_right_plf[k].write(dataRightInput[k],tb.instance_active_elements_right(k)*sizeof(float), 0); });
      main_queue[k].enqueue(right_in_finished[k]);

      // time: t1
      main_queue[k].enqueue([&execution_ms, k, i, t] {execution_ms[k].t1[i] = t.elapsed();});

      // execute acceleration platform
      execution_finished[k] = main_queue[k].enqueue([&mm2sleft_run, &mm2sright_run, &s2mm_run, k] {s2mm_run[k].start(); mm2sleft_run[k].start(); mm2sright_run[k].start(); mm2sleft_run[k].wait(); mm2sright_run[k].wait(); s2mm_run[k].wait();});
      right_queue[k].enqueue(execution_finished[k]);

      // time: t2
      main_queue[k].enqueue([&execution_ms, k, i, t] {execution_ms[k].t2[i] = t.elapsed();});


      // read data back from Versal
      main_queue[k].enqueue([&out_plf, dataOutput, &tb, k] {out_plf[k].read(dataOutput, sizeof(float)*tb.alignmentelements_per_instance(k), 0); });
      right_out_finished[k] = right_queue[k].enqueue([&out_scaler, scaler, &tb, k] {out_scaler[k].read(scaler, sizeof(char)*tb.alignments_per_instance(k),0); });
      main_queue[k].enqueue(right_out_finished[k]);

      // time: end
      instance_done[k] = main_queue[k].enqueue([&execution_ms, k, i, t] {execution_ms[k].end[i] = t.elapsed();});

    }

    // Sync all threads
    for (unsigned int k = 0; k < tb.parallel_instances; k++) {
      instance_done[k].wait();
    }

#else
    execution_ms.begin[i] = t.elapsed();

    for (unsigned long int k = 0; k < tb.parallel_instances; k++) {
      unsigned int alignment_offset = k*tb.alignmentelements_per_instance(0);

      std::copy(ev,                ev+16,                                          dataLeftInput[k]      );
      std::copy(branchleft,        branchleft+64,                                  dataLeftInput[k] + 16 );
      std::copy(&alignmentsleft[alignment_offset], &alignmentsleft[alignment_offset + tb.alignmentelements_per_instance(k)], dataLeftInput[k] + 80 );
      if (tb.input_layout == COMBINED) {
        std::copy(ev,                 ev+16,                                           dataRightInput[k]      );
        std::copy(branchright,        branchright+64,                                  dataRightInput[k] + 16 );
        std::copy(&alignmentsright[alignment_offset], &alignmentsright[alignment_offset + tb.alignmentelements_per_instance(k)], dataRightInput[k] + 80 );
      } else {
        std::copy(branchright,        branchright+64,                                  dataRightInput[k]      );
        std::copy(&alignmentsright[alignment_offset], &alignmentsright[alignment_offset + tb.alignmentelements_per_instance(k)], dataRightInput[k] + 64 );
      }
    }

    execution_ms.t1[i] = t.elapsed();

    // Execute PLF calculation parallelized on Versal
    for (unsigned int k = 0; k < tb.parallel_instances; k++) {

      // set pointers for lamda functions
      scaler = scalerVector_versal[i] + k*tb.alignments_per_instance(0);
      dataOutput = versalResult[i] + (k*tb.alignmentelements_per_instance(0));

      // write data to Versal
      main_queue[k].enqueue([&in_left_plf, &dataLeftInput, &tb, k] {in_left_plf[k].write(dataLeftInput[k], tb.instance_active_elements_left(k)*sizeof(float), 0); });
      right_queue[k].enqueue([&in_right_plf, &dataRightInput, &tb, k] {in_right_plf[k].write(dataRightInput[k],tb.instance_active_elements_right(k)*sizeof(float), 0); });


      // execute acceleration platform
      main_queue[k].enqueue([&mm2sleft_run, k] {mm2sleft_run[k].start(); mm2sleft_run[k].wait();});
      right_done[k] = right_queue[k].enqueue([&mm2sright_run, k] {mm2sright_run[k].start(); mm2sright_run[k].wait();});
      execution_finished[k] = output_queue[k].enqueue([&s2mm_run, k] {s2mm_run[k].start(); s2mm_run[k].wait();});


      // read data back from Versal
      main_queue[k].enqueue(right_done[k]);
      main_queue[k].enqueue(execution_finished[k]);
      main_queue[k].enqueue([&out_scaler, scaler, &tb, k] {out_scaler[k].read(scaler, sizeof(char)*tb.alignments_per_instance(k), 0); });
      output_done[k] = output_queue[k].enqueue([&out_plf, dataOutput, &tb, k] {out_plf[k].read(dataOutput, sizeof(float)*tb.alignmentelements_per_instance(k), 0); });
      instance_done[k] = main_queue[k].enqueue(output_done[k]);

    }

    // Sync all threads
    for (unsigned int k = 0; k < tb.parallel_instances; k++) {
      instance_done[k].wait();
    }

    execution_ms.t2[i] = t.elapsed();

#endif

    // Calculate scalerIncrement
    scalerIncrement_versal[i] = 0;
    for (unsigned int j = 0; j < tb.alignment_sites; j++) {
      scalerIncrement_versal[i] += scalerVector_versal[i][j] * wgt[j];
    }

#if defined(NO_INTERMEDIATE_RESULTS) && NO_INTERMEDIATE_RESULTS == 1
    execution_ms.end[i] = t.elapsed();
#endif

  }
  xrt_profile.end();

  //Check///////////////////////////////////////////////////////////////////////////////////////////////////

  timing_data reference_ms;
  reference_ms.init(tb.plf_calls);
  //t.reset();

#if !defined(NO_CORRECTNESS_CHECK) || NO_CORRECTNESS_CHECK == 0
  std::cout << "Data collected, checking for correctness ..." << std::endl;
  // Test correctness of return data
  std::string result = "Passed";
  unsigned int errors = 0;

  int scalerIncrement_cpu[tb.plf_calls] = {0};

  float** cpuResult = new float*[tb.plf_calls];
  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    cpuResult[i] = new float[tb.elements_per_plf()];
  }

  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    reference_ms.t1[i] = t.elapsed();
    plf(alignmentsleft, alignmentsright, cpuResult[i], ev, tb.alignment_sites, branchleft, branchright, wgt, scalerIncrement_cpu[i]);
    reference_ms.t2[i] = t.elapsed();
  }
  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    for (unsigned long int j = 0; j < tb.elements_per_plf(); j++) {
      if (cpuResult[i][j] != versalResult[i][j]) {
        std::cout << "ERROR: alignment data wrong for call " << i << " at alignment " << (j>>4) << ", probability " << (j%16) << ", cpu!=versal: " << cpuResult[i][j] << "!=" << versalResult[i][j] << std::endl;
        errors++;
        result = " Failed with " + std::to_string(errors) + " errors";
        if (errors >= 20) {
          result = " Failed with more than 20 errors";
          break;
        }
      }
    }
  }

  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    if (scalerIncrement_cpu[i]!=(int)scalerIncrement_versal[i]) {
      std::cout << "ERROR: scalerIncrement wrong for call " << i << ", cpu!=versal: " << scalerIncrement_cpu[i] << "!=" << scalerIncrement_versal[i] << std::endl;
    }
  }
  std::cout << std::endl;
  std::cout << "Test result: " << result << std::endl;
#endif


  //Result//////////////////////////////////////////////////////////////////////////////////////////////////

  // TODO: print results for all threads in a table
#if !defined(NO_INTERMEDIATE_RESULTS) || NO_INTERMEDIATE_RESULTS == 0
  print_timing_data(execution_ms[0], reference_ms, (double)tb.data_size(), tb.alignment_sites * tb.plf_calls, tb.plf_calls);
  if (acap.get_target() == "hw") {
    std::string csvFile = std::string("data_hw_runs/") + acap.get_app_name() + "_" + acap.get_aie_name() + "_" + acap.get_pl_name() + "_plfs" + std::to_string(tb.plf_calls) + "_alignments" + std::to_string(tb.alignment_sites) + "_usedgraphs" + std::to_string(tb.parallel_instances) + ".csv";
    write_to_csv(csvFile, execution_ms, tb.parallel_instances);
  }
#else
  std::cout << std::endl;
  std::cout << "=====================================================================================================" << std::endl;
  std::cout << "| Timing region                          | time (ms)  | bandwidth (MB/s) |         bandwidth (MA/s) |" << std::endl;
  std::cout << "=====================================================================================================" << std::endl;
  std::cout << "| Prepare input for Versal:              | " << std::setw(10) << execution_ms.hm() << " | ";
  std::cout << std::setw(16) << bandwidth_MBs(execution_ms.hm(), (double)tb.data_size())    << " | ";
  std::cout << std::setw(24) << bandwidth_As(execution_ms.hm(), tb.alignment_sites*tb.plf_calls) * 1e-6 << " |" << std::endl;
  std::cout << "| PLF on Versal:                         | " << std::setw(10) << execution_ms.msm() << " | ";
  std::cout << std::setw(16) << bandwidth_MBs(execution_ms.msm(), (double)tb.data_size())    << " | ";
  std::cout << std::setw(24) << bandwidth_As(execution_ms.msm(), tb.alignment_sites*tb.plf_calls) * 1e-6 << " |" << std::endl;
  std::cout << "| scaling wgt mult:                      | " << std::setw(10) << execution_ms.mh() << " | ";
  std::cout << std::setw(16) << bandwidth_MBs(execution_ms.mh(), (double)tb.data_size())    << " | ";
  std::cout << std::setw(24) << bandwidth_As(execution_ms.mh(), tb.alignment_sites*tb.plf_calls) * 1e-6 << " |" << std::endl;
  std::cout << "=====================================================================================================" << std::endl;
  if (acap.get_target() == "hw") {
    std::string csvFile = std::string("data_hw_runs/") + acap.get_app_name() + "_" + acap.get_aie_name() + "_" + acap.get_pl_name() + "_plfs" + std::to_string(tb.plf_calls) + "_alignments" + std::to_string(tb.alignment_sites) + "_usedgraphs" + std::to_string(tb.parallel_instances) + ".csv";
    write_to_csv(csvFile, execution_ms);
  }
#endif



  //Cleanup////////////////////////////////////////////////////////////////////////////////////////////////


  std::cout << "delete timing struct" << std::endl;
#if !defined(NO_INTERMEDIATE_RESULTS) || NO_INTERMEDIATE_RESULTS == 0
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    execution_ms[i].destroy();
  }
  reference_ms.destroy();
  delete[] execution_ms;
  execution_ms = nullptr;
#else
  execution_ms.destroy();
#endif

  std::cout << "delete inner data arrays" << std::endl;
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    delete[] dataLeftInput[i];
    delete[] dataRightInput[i];
  }
  for (unsigned int i = 0; i < tb.plf_calls; i++) {
    delete[] scalerVector_versal[i];
    delete[] versalResult[i];
#if !defined(NO_CORRECTNESS_CHECK) || NO_CORRECTNESS_CHECK == 0
    delete[] cpuResult[i];
#endif
  }

  std::cout << "delete data arrays" << std::endl;
  delete[] dataLeftInput;
  delete[] dataRightInput;
  delete[] versalResult;
#if !defined(NO_CORRECTNESS_CHECK) || NO_CORRECTNESS_CHECK == 0
  delete[] cpuResult;
#endif
  delete[] scalerVector_versal;
  delete[] scalerIncrement_versal;
  delete[] wgt;
  delete[] alignmentsleft;
  delete[] alignmentsright;
  dataLeftInput = nullptr;
  dataRightInput = nullptr;
  versalResult = nullptr;
#if !defined(NO_CORRECTNESS_CHECK) || NO_CORRECTNESS_CHECK == 0
  cpuResult = nullptr;
#endif
  alignmentsleft = nullptr;
  alignmentsright = nullptr;

  std::cout << "delete handles" << std::endl;
  delete[] mm2sleft_kernels;
  delete[] mm2sright_kernels;
  delete[] s2mm_kernels;
  delete[] in_left_plf;
  delete[] in_right_plf;
  delete[] out_plf;
  delete[] mm2sleft_run;
  delete[] mm2sright_run;
  delete[] s2mm_run;
  mm2sleft_kernels = nullptr;
  mm2sright_kernels = nullptr;
  s2mm_kernels = nullptr;
  in_left_plf = nullptr;
  in_right_plf = nullptr;
  out_plf = nullptr;
  mm2sleft_run = nullptr;
  mm2sright_run = nullptr;
  s2mm_run = nullptr;

  return 0;
}
