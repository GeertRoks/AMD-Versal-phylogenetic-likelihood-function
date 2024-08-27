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
  tb.window_size = 1024;
  tb.input_layout = acap.classifyLayoutType(acap.get_pl_name());

  tb.aie_type = acap.classifyAieType(acap.get_pl_name());

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
    in_left_plf[i] = xrt::bo(*acap.get_device(), tb.instance_size_left(), xrt::bo::flags::normal, mm2sleft_kernels[i].group_id(0));
    in_right_plf[i] = xrt::bo(*acap.get_device(), tb.instance_size_right(), xrt::bo::flags::normal, mm2sright_kernels[i].group_id(0));
    out_plf[i] = xrt::bo(*acap.get_device(), tb.instance_size_out(), xrt::bo::flags::normal, s2mm_kernels[i].group_id(0));
    out_scaler[i] = xrt::bo(*acap.get_device(), sizeof(int), xrt::bo::flags::normal, s2mm_kernels[i].group_id(1));
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
  float* alignmentsleft_cpu = new float[tb.elements_per_plf()];
  float* alignmentsright_cpu = new float[tb.elements_per_plf()];

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
    alignmentsleft_cpu[j] = dis(gen) * scale;
    //std::cout << "scale: " << scale << "left: " << alignmentsleft_cpu[j] << std::endl;
    alignmentsright_cpu[j] = dis(gen);
  }

  int wgt[tb.alignment_sites] = {0};
  unsigned int scalerIncrement_versal[tb.plf_calls][tb.parallel_instances] = {0};

  float** versalResult = new float*[tb.plf_calls];
  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    versalResult[i] = new float[tb.elements_per_plf()];
  }

  std::cout << "Prepare data for transfer ... " << std::endl;

  float** alignmentsleft = new float*[tb.parallel_instances];
  float** alignmentsright = new float*[tb.parallel_instances];

  float** dataLeftInput = new float*[tb.parallel_instances];
  float** dataRightInput = new float*[tb.parallel_instances];
  float** dataOutput = new float*[tb.parallel_instances];

  for (unsigned long int i = 0; i < tb.parallel_instances; i++) {
    alignmentsleft[i] = new float[tb.elements_per_instance()];
    alignmentsright[i] = new float[tb.elements_per_instance()];

    dataLeftInput[i] = new float[tb.instance_elements_left()];
    dataRightInput[i] = new float[tb.instance_elements_right()];
    dataOutput[i] = new float[tb.instance_elements_out()];

    // load alignment data
    for (unsigned long int j = 0; j < tb.alignmentelements_per_instance(i); j++) {
      alignmentsleft[i][j] = alignmentsleft_cpu[tb.alignmentelements_per_instance(0)*i + j];
      alignmentsright[i][j] = alignmentsright_cpu[tb.alignmentelements_per_instance(0)*i + j];
    }

    std::copy(ev,                ev+16,                                          dataLeftInput[i]      );
    std::copy(branchleft,        branchleft+64,                                  dataLeftInput[i] + 16 );
    std::copy(alignmentsleft[i], alignmentsleft[i] + tb.elements_per_instance(), dataLeftInput[i] + 80 );
    if (tb.input_layout == ONE_INEV) {
      std::copy(ev,                 ev+16,                                           dataRightInput[i]      );
      std::copy(branchright,        branchright+64,                                  dataRightInput[i] + 16 );
      std::copy(alignmentsright[i], alignmentsright[i] + tb.elements_per_instance(), dataRightInput[i] + 80 );
    } else {
      std::copy(branchright,        branchright+64,                                  dataRightInput[i]      );
      std::copy(alignmentsright[i], alignmentsright[i] + tb.elements_per_instance(), dataRightInput[i] + 64 );
    }
  }


  // load wgt to s2mm
  for (unsigned long int i = 0; i < tb.alignment_sites; i++) {
    wgt[i] = 1;
  }
  //std::copy(wgt, wgt + tb.alignment_sites, dataOutput[0]);

  //out_buffer.write(dataOutput[0]);
  //out_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);
//for (unsigned long int j = 0; j < tb.parallel_instances; j++) {
  //  s2mm_run[j].set_arg(0, out_buffer);
  //  s2mm_run[j].set_arg(3, 1);
  //}
  //for (unsigned int k = 0; k < tb.parallel_instances; k++) {
  //  s2mm_run[k].start();
  //}
  //for (unsigned int k = 0; k < tb.parallel_instances; k++) {
  //  s2mm_run[k].wait();
  //}

  //// reset s2mm param for execution
  //for (unsigned long int j = 0; j < tb.parallel_instances; j++) {
  //  s2mm_run[j].set_arg(0, out_plf[j]);
  //  s2mm_run[j].set_arg(3, 0);
  //}


  //Run///////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "Setup data gathering structures ... " << std::endl << std::endl;
  xrt::queue main_queue[tb.parallel_instances];
  xrt::queue right_queue[tb.parallel_instances];
  xrt::queue::event right_finished[tb.parallel_instances];
  xrt::queue::event done[tb.parallel_instances];


  auto xrt_profile = xrt::profile::user_range("roundtrip_exec_time", "The execution time of the full test");
  timing_data* execution_ms = new timing_data[tb.parallel_instances];
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    execution_ms[i].init(tb.plf_calls);
  }
  Timer t;

  unsigned int* scaler;

  std::cout << "Start PLF calculation on accelerator ... " << std::endl << std::endl;
  //t.reset();

  xrt_profile.start("roundtrip_exec_time");
  for (long int i = 0; i < tb.plf_calls; i++) {
    //TODO: add timer to measure one call that includes overhead of copying the output

    scaler = scalerIncrement_versal[i];

    for (unsigned int k = 0; k < tb.parallel_instances; k++) {

      // time: begin
      main_queue[k].enqueue([&execution_ms, k, i, t] {execution_ms[k].begin[i] = t.elapsed();});

      // write data to Versal
      main_queue[k].enqueue([&in_left_plf, &dataLeftInput, k] {in_left_plf[k].write(dataLeftInput[k]); in_left_plf[k].sync(XCL_BO_SYNC_BO_TO_DEVICE); });
      right_finished[k] = right_queue[k].enqueue([&in_right_plf, &dataRightInput, k] {in_right_plf[k].write(dataRightInput[k]); in_right_plf[k].sync(XCL_BO_SYNC_BO_TO_DEVICE); });
      main_queue[k].enqueue(right_finished[k]);


      // time: t1
      main_queue[k].enqueue([&execution_ms, k, i, t] {execution_ms[k].t1[i] = t.elapsed();});

      // execute acceleration platform
      main_queue[k].enqueue([&mm2sleft_run, &mm2sright_run, &s2mm_run, k] {s2mm_run[k].start(); mm2sleft_run[k].start(); mm2sright_run[k].start(); mm2sleft_run[k].wait(); mm2sright_run[k].wait(); s2mm_run[k].wait();});

      // time: t2
      main_queue[k].enqueue([&execution_ms, k, i, t] {execution_ms[k].t2[i] = t.elapsed();});


      // read data back from Versal
      main_queue[k].enqueue([&out_plf, &dataOutput, k] {out_plf[k].sync(XCL_BO_SYNC_BO_FROM_DEVICE); out_plf[k].read(dataOutput[k]);});
      right_finished[k] = right_queue[k].enqueue([&out_scaler, &scaler, k] {out_scaler[k].sync(XCL_BO_SYNC_BO_FROM_DEVICE); out_scaler[k].read(&scaler[k]);});
      main_queue[k].enqueue(right_finished[k]);

      // time: end
      done[k] = main_queue[k].enqueue([&execution_ms, k, i, t] {execution_ms[k].end[i] = t.elapsed();});

    }


    // Sync all threads
    for (unsigned int k = 0; k < tb.parallel_instances; k++) {
      done[k].wait();
      std::copy(dataOutput[k], dataOutput[k]+tb.alignmentelements_per_instance(k), versalResult[i]+(k*tb.alignmentelements_per_instance(0)));
    }

  }
  xrt_profile.end();

  //Check///////////////////////////////////////////////////////////////////////////////////////////////////
  std::cout << "Data collected, checking for correctness ..." << std::endl;

  timing_data reference_ms;
  reference_ms.init(tb.plf_calls);
  //t.reset();

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
    plf(alignmentsleft_cpu, alignmentsright_cpu, cpuResult[i], ev, tb.alignment_sites, branchleft, branchright, wgt, scalerIncrement_cpu[i]);
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

  // TODO: add scaler output check back in
  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    unsigned int scalerInc = 0;
    for (unsigned int j = 0; j < tb.parallel_instances; j++) {
      scalerInc += scalerIncrement_versal[i][j];
      std::cout << "scalerIncrement[" << i << "][" << j << "]: " << scalerIncrement_versal[i][j] << std::endl;
    }
    if (scalerIncrement_cpu[i]!=(int)scalerInc) {
      std::cout << "ERROR: scalerIncrement wrong for call " << i << ", cpu!=versal: " << scalerIncrement_cpu[i] << "!=" << scalerInc << std::endl;
    }
  }


  //Result//////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << std::endl;
  std::cout << "Test result: " << result << std::endl;
  // TODO: print results for all threads in a table
  print_timing_data(execution_ms[0], reference_ms, (double)tb.data_size(), tb.alignment_sites * tb.plf_calls, tb.plf_calls);


  if (acap.get_target() == "hw") {
    std::string csvFile = std::string("data_hw_runs/") + acap.get_app_name() + "_" + acap.get_aie_name() + "_" + acap.get_pl_name() + "_plfs" + std::to_string(tb.plf_calls) + "_alignments" + std::to_string(tb.alignment_sites) + "_usedgraphs" + std::to_string(tb.parallel_instances) + ".csv";
    write_to_csv(csvFile, execution_ms, tb.parallel_instances);
  }

  //Cleanup////////////////////////////////////////////////////////////////////////////////////////////////


  std::cout << "delete timing struct" << std::endl;
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    execution_ms[i].destroy();
  }
  reference_ms.destroy();
  delete[] execution_ms;
  execution_ms = nullptr;

  std::cout << "delete inner data arrays" << std::endl;
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    delete[] dataLeftInput[i];
    delete[] dataRightInput[i];
    delete[] dataOutput[i];
    delete[] alignmentsleft[i];
    delete[] alignmentsright[i];
  }
  for (unsigned int i = 0; i < tb.plf_calls; i++) {
    delete[] versalResult[i];
    delete[] cpuResult[i];
  }

  std::cout << "delete data arrays" << std::endl;
  delete[] dataLeftInput;
  delete[] dataRightInput;
  delete[] dataOutput;
  delete[] versalResult;
  delete[] cpuResult;
  delete[] alignmentsleft;
  delete[] alignmentsright;
  delete[] alignmentsleft_cpu;
  delete[] alignmentsright_cpu;
  dataLeftInput = nullptr;
  dataRightInput = nullptr;
  dataOutput = nullptr;
  versalResult = nullptr;
  cpuResult = nullptr;
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
