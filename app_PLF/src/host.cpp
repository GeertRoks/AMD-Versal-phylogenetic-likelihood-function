#define STRINGIFY_BACKEND(A) #A
#define STRINGIFY(A) STRINGIFY_BACKEND(A)

#include "include.h"
#include "timing.h"
#include "data.h"

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
  xrt::bo in_left_buffer = xrt::bo(*acap.get_device(), tb.buffer_size_left(), xrt::bo::flags::normal, mm2sleft_kernels[0].group_id(0));
  xrt::bo in_right_buffer = xrt::bo(*acap.get_device(), tb.buffer_size_right(), xrt::bo::flags::normal, mm2sright_kernels[0].group_id(0));
  xrt::bo out_buffer = xrt::bo(*acap.get_device(), tb.buffer_size_out(), xrt::bo::flags::normal, s2mm_kernels[0].group_id(0));
  xrt::bo bo_scalerIncrement = xrt::bo(*acap.get_device(), tb.parallel_instances * sizeof(int), xrt::bo::flags::normal, s2mm_kernels[0].group_id(1));

  xrt::bo* in_left_plf = new xrt::bo[tb.parallel_instances];
  xrt::bo* in_right_plf = new xrt::bo[tb.parallel_instances];
  xrt::bo* out_plf = new xrt::bo[tb.parallel_instances];
  xrt::bo* bo_scaler = new xrt::bo[tb.parallel_instances];
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    in_left_plf[i] = xrt::bo(in_left_buffer, tb.instance_size_left(), i*tb.instance_size_left());
    in_right_plf[i] = xrt::bo(in_right_buffer, tb.instance_size_right(), i*tb.instance_size_right());
    out_plf[i] = xrt::bo(out_buffer, tb.instance_size_out(), i*tb.instance_size_out());
    bo_scaler[i] = xrt::bo(bo_scalerIncrement, sizeof(int), i*sizeof(int));
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
    mm2sleft_run[i].set_arg(1, tb.alignments_per_instance() - ((i == tb.parallel_instances-1) * tb.alignments_padding()));
    //mm2sleft_run[i].set_arg(1, tb.alignments_per_instance());
    mm2sleft_run[i].set_arg(2, tb.window_size);

    mm2sright_run[i] = xrt::run(mm2sright_kernels[i]);
    mm2sright_run[i].set_arg(0, in_right_plf[i]);
    mm2sright_run[i].set_arg(1, tb.alignments_per_instance() - ((i == tb.parallel_instances-1) * tb.alignments_padding()));
    //mm2sright_run[i].set_arg(1, tb.alignments_per_instance());
    mm2sright_run[i].set_arg(2, tb.window_size);

    s2mm_run[i] = xrt::run(s2mm_kernels[i]);
    s2mm_run[i].set_arg(0, out_plf[i]);
    s2mm_run[i].set_arg(1, bo_scaler[i]);
    s2mm_run[i].set_arg(2, tb.alignments_per_instance() - ((i == tb.parallel_instances-1) * tb.alignments_padding()));
    //s2mm_run[i].set_arg(2, tb.alignments_per_instance());
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

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0f, 1.0f);

  float** branchleft = new float*[tb.plf_calls];
  float** branchright = new float*[tb.plf_calls];
  float** ev = new float*[tb.plf_calls];
  float** alignmentsleft = new float*[tb.plf_calls];
  float** alignmentsright = new float*[tb.plf_calls];

  int wgt[tb.alignment_sites] = {0};
  unsigned int scalerIncrement_versal[tb.plf_calls][tb.parallel_instances] = {0};

  float** dataLeftInput = new float*[tb.plf_calls];
  float** dataRightInput = new float*[tb.plf_calls];
  float** dataOutput = new float*[tb.plf_calls];

  float** checkOutput = new float*[tb.plf_calls];

  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    branchleft[i] = new float[64];
    branchright[i] = new float[64];
    ev[i] = new float[16];
    alignmentsleft[i] = new float[tb.elements_per_plf() + tb.alignments_padding_elements()];
    alignmentsright[i] = new float[tb.elements_per_plf() + tb.alignments_padding_elements()];

    dataOutput[i] = new float[tb.buffer_elements_out()];
    checkOutput[i] = new float[tb.buffer_elements_out()];
    dataLeftInput[i] = new float[tb.buffer_elements_left()];
    dataRightInput[i] = new float[tb.buffer_elements_right()];

    // load ev
    for (unsigned long int j = 0; j < 16; j++) {
      ev[i][j] = dis(gen);
    }
    // load branch left and branch right
    for (unsigned long int j = 0; j < 64; j++) {
      branchleft[i][j] = dis(gen);
      branchright[i][j] = dis(gen);
    }
    // load alignment data
    for (unsigned long int j = 0; j < tb.elements_per_plf(); j++) {
      alignmentsleft[i][j] = dis(gen) * 1.0e-12;
      alignmentsright[i][j] = dis(gen);
    }

    for (unsigned long int j = 0; j < tb.parallel_instances; j++) {
      std::copy(ev[i],                                               ev[i]+16,                                              dataLeftInput[i] + j*tb.instance_elements_left()          );
      std::copy(branchleft[i],                                       branchleft[i]+64,                                      dataLeftInput[i] + 16 + j*tb.instance_elements_left()     );
      std::copy(alignmentsleft[i] + j*tb.alignments_per_instance(j)*tb.elements_per_alignment,    alignmentsleft[i] + (j+1)*tb.alignments_per_instance(j)*tb.elements_per_alignment,  dataLeftInput[i] + 80 + j * tb.instance_elements_left()   );
      if (tb.input_layout == ONE_INEV) {
        std::copy(ev[i],                                             ev[i]+16,                                              dataRightInput[i] + j*tb.instance_elements_right()        );
        std::copy(branchright[i],                                    branchright[i]+64,                                     dataRightInput[i] + 16 + j*tb.instance_elements_right()   );
        std::copy(alignmentsright[i] + j*tb.alignments_per_instance(j)*tb.elements_per_alignment, alignmentsright[i] + (j+1)*tb.alignments_per_instance(j)*tb.elements_per_alignment, dataRightInput[i] + 80 + j * tb.instance_elements_right() );
      } else {
        std::copy(branchright[i],                                    branchright[i]+64,                                     dataRightInput[i] + j*tb.instance_elements_right()        );
        std::copy(alignmentsright[i] + j*tb.alignments_per_instance(j)*tb.elements_per_alignment, alignmentsright[i] + (j+1)*tb.alignments_per_instance(j)*tb.elements_per_alignment, dataRightInput[i] + 64 + j * tb.instance_elements_right() );
      }
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
  auto xrt_profile = xrt::profile::user_range("roundtrip_exec_time", "The execution time of the full test");
  Timer t;
  timing_data execution_ms(tb.plf_calls);


  std::cout << "Start PLF calculation on accelerator ... " << std::endl;
  //t.reset();
  xrt_profile.start("roundtrip_exec_time");
  for (long int i = 0; i < tb.plf_calls; i++) {
    execution_ms.begin[i] = t.elapsed();

    // Move data from host to ACAP Memory
    in_left_buffer.write(dataLeftInput[i]);
    in_right_buffer.write(dataRightInput[i]);

    in_left_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    in_right_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    execution_ms.t1[i] = t.elapsed();

    for (unsigned int k = 0; k < tb.parallel_instances; k++) {
      s2mm_run[k].start();
      mm2sleft_run[k].start();
      mm2sright_run[k].start();
    }
    for (unsigned int k = 0; k < tb.parallel_instances; k++) {
      mm2sleft_run[k].wait();
      mm2sright_run[k].wait();
      s2mm_run[k].wait();
    }

    execution_ms.t2[i] = t.elapsed();

    // Move data from ACAP Memory to host
    out_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    bo_scalerIncrement.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    out_buffer.read(dataOutput[i]);
    bo_scalerIncrement.read(scalerIncrement_versal[i]);

    execution_ms.end[i] = t.elapsed();
  }
  xrt_profile.end();

  //Check///////////////////////////////////////////////////////////////////////////////////////////////////
  std::cout << "Data collected, checking for correctness ..." << std::endl;


  timing_data reference_ms(tb.plf_calls);
  t.reset();

  // Test correctness of return data
  std::string result = "Passed";
  unsigned int errors = 0;

  int scalerIncrement_cpu[tb.plf_calls] = {0};

  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    reference_ms.t1[i] = t.elapsed();
    plf(alignmentsleft[i], alignmentsright[i], checkOutput[i], ev[i], tb.alignment_sites, branchleft[i], branchright[i], wgt, scalerIncrement_cpu[i]);
    reference_ms.t2[i] = t.elapsed();
  }

  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    unsigned int scalerInc = 0;
    for (unsigned int j = 0; j < tb.parallel_instances; j++) {
      scalerInc += scalerIncrement_versal[i][j];
    }
    if (scalerIncrement_cpu[i]!=(int)scalerInc) {
      std::cout << "ERROR: scalerIncrement wrong for call " << i << ", cpu!=versal: " << scalerIncrement_cpu[i] << "!=" << scalerInc << std::endl;
    }
  }
  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    for (unsigned long int j = 0; j < tb.parallel_instances; j++) {
      for (unsigned long int k = 0; k < tb.alignments_per_instance(j)*tb.elements_per_alignment; k++) {
        if(dataOutput[i][(j*tb.instance_elements_out())+k]!=checkOutput[i][(j*tb.alignments_per_instance(j)*tb.elements_per_alignment)+k]) {
          std::cout << "ERROR: alignment data wrong at block: " << i << ", index: " << j << ", cpu!=versal: " << checkOutput[i][(j*tb.alignments_per_instance(j)*tb.elements_per_alignment)+k] << "!=" << dataOutput[i][(j*tb.instance_elements_out())+k] << std::endl;
          errors++;
          result = " Failed with " + std::to_string(errors) + " errors";
          if (errors >= 20) {
            result = " Failed with more than 20 errors";
            break;
          }
        }
      }
    }
  }

  //Result//////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << std::endl;
  std::cout << "Test result: " << result << std::endl;
  print_timing_data(execution_ms, reference_ms, (double)tb.data_size(), tb.alignment_sites * tb.plf_calls, tb.plf_calls);


  if (acap.get_target() == "hw") {
    std::string csvFile = std::string("data_hw_runs/") + acap.get_app_name() + "_" + acap.get_aie_name() + "_" + acap.get_pl_name() + "_plfs" + std::to_string(tb.plf_calls) + "_alignments" + std::to_string(tb.alignment_sites) + "_usedgraphs" + std::to_string(tb.parallel_instances) + ".csv";
    write_to_csv(csvFile, execution_ms);
  }

  //Cleanup////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "delete inner data arrays" << std::endl;
  for (unsigned int i = 0; i < tb.plf_calls; i++) {
    delete[] dataLeftInput[i];
    delete[] dataRightInput[i];
    delete[] dataOutput[i];
    delete[] checkOutput[i];
    delete[] branchleft[i];
    delete[] branchright[i];
    delete[] ev[i];
    delete[] alignmentsleft[i];
    delete[] alignmentsright[i];
  }

  std::cout << "delete data arrays" << std::endl;
  delete[] dataLeftInput;
  delete[] dataRightInput;
  delete[] dataOutput;
  delete[] checkOutput;
  delete[] branchleft;
  delete[] branchright;
  delete[] ev;
  delete[] alignmentsleft;
  delete[] alignmentsright;
  dataLeftInput = nullptr;
  dataRightInput = nullptr;
  dataOutput = nullptr;
  checkOutput = nullptr;
  branchleft = nullptr;
  branchright = nullptr;
  ev = nullptr;
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
