#define STRINGIFY_BACKEND(A) #A
#define STRINGIFY(A) STRINGIFY_BACKEND(A)

#include "include.h"
#include "timing.h"
#include "data.h"

int main(int argc, char* argv[]) {

  if(argc != 2)
    throw std::runtime_error(std::string("Not correct amount of parameters provided. Usage: ") + argv[0] + " </path/to/a.xclbin>\n");

  acap_info acap(argv[1]);
  testbench_info tb;
  tb.alignment_sites=10000;
  tb.plf_calls = 1;
  tb.window_size = 1024;
  tb.combined_ev = 0;

  std::cout << std::endl;
  std::cout << "=======================================================================" << std::endl;
  std::cout << "| test name:              | " << acap.get_app_name() << std::endl;
  std::cout << "| PL name:                | " << acap.get_pl_name() << std::endl;
  std::cout << "| AIE name:               | " << acap.get_aie_name() << std::endl;
  std::cout << "=======================================================================" << std::endl;
  std::cout << "| parallel plfs:          | " << tb.parallel_instances << std::endl;
  std::cout << "| plf calls per instance: | " << tb.plf_calls << std::endl;
  std::cout << "-----------------------------------------------------------------------" << std::endl;
  std::cout << "| alignment sites:        | " << tb.alignment_sites << std::endl;
  std::cout << "| data elements per site: | " << tb.elements_per_alignment << std::endl;
  std::cout << "| data elements per plf:  | " << tb.elements_per_plf() << std::endl;
  std::cout << "| AIE window size:        | " << tb.window_size << std::endl;
  std::cout << "-----------------------------------------------------------------------" << std::endl;
  std::cout << "| data size:              | " << tb.data_size() << std::endl;
  std::cout << "| data elements:          | " << tb.data_elements() << std::endl;
  std::cout << "| word size:              | " << tb.word_size << std::endl;
  std::cout << "-----------------------------------------------------------------------" << std::endl;
  std::cout << "| buffer size left:       | " << tb.buffer_size_left() << std::endl;
  std::cout << "| buffer size right:      | " << tb.buffer_size_right() << std::endl;
  std::cout << "| buffer size out:        | " << tb.buffer_size_out() << std::endl;
  std::cout << "-----------------------------------------------------------------------" << std::endl;
  std::cout << "| buffer elements left:   | " << tb.buffer_elements_left() << std::endl;
  std::cout << "| buffer elements right:  | " << tb.buffer_elements_right() << std::endl;
  std::cout << "| buffer elements out:    | " << tb.buffer_elements_out() << std::endl;
  std::cout << "=======================================================================" << std::endl;
  std::cout << "| device name:            | " << acap.get_device()->get_info<xrt::info::device::name>() << std::endl;
  std::cout << "| device bdf:             | " << acap.get_device()->get_info<xrt::info::device::bdf>() << std::endl;
  std::cout << "-----------------------------------------------------------------------" << std::endl;
  std::cout << "| xclbin host_type:       | " << acap.get_target() << std::endl;
  std::cout << "| xclbin kernels:         | ";
  for (auto kernel : acap.get_kernels()) {
    std::cout << kernel.get_name() << " ";
  }
  std::cout << std::endl;
  //std::cout << "| xclbin uuid:         | " << xclbin.get_uuid().to_string() << std::endl;
  std::cout << "=======================================================================" << std::endl;
  std::cout << std::endl;



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
  }
  printf("\nconnected to datamover kernels\n");

  // Create Host <-> ACAP Memory buffers
  xrt::bo in_left_buffer = xrt::bo(*acap.get_device(), tb.buffer_size_left(), xrt::bo::flags::normal, mm2sleft_kernels[0].group_id(0));
  xrt::bo in_right_buffer = xrt::bo(*acap.get_device(), tb.buffer_size_right(), xrt::bo::flags::normal, mm2sright_kernels[0].group_id(0));
  xrt::bo out_buffer = xrt::bo(*acap.get_device(), tb.buffer_size_out(), xrt::bo::flags::normal, s2mm_kernels[0].group_id(0));

  // Create the HLS kernel run handles and set the parameters
  xrt::run* mm2sleft_run = new xrt::run[tb.parallel_instances];
  xrt::run* mm2sright_run = new xrt::run[tb.parallel_instances];
  xrt::run* s2mm_run = new xrt::run[tb.parallel_instances];
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    mm2sleft_run[i] = xrt::run(mm2sleft_kernels[i]);
    mm2sleft_run[i].set_arg(0, in_left_buffer);
    mm2sleft_run[i].set_arg(1, tb.alignment_sites);
    mm2sleft_run[i].set_arg(2, tb.window_size);

    mm2sright_run[i] = xrt::run(mm2sright_kernels[i]);
    mm2sright_run[i].set_arg(0, in_right_buffer);
    mm2sright_run[i].set_arg(1, tb.alignment_sites);
    mm2sright_run[i].set_arg(2, tb.window_size);

    s2mm_run[i] = xrt::run(s2mm_kernels[i]);
    s2mm_run[i].set_arg(0, out_buffer);
    s2mm_run[i].set_arg(1, tb.alignment_sites);
    s2mm_run[i].set_arg(2, tb.window_size);
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

  //Run///////////////////////////////////////////////////////////////////////////////////////////////////

  auto xrt_profile = xrt::profile::user_range("roundtrip_exec_time", "The execution time of the full test");

  Timer t;
  timing_data execution_ms(tb.plf_calls);

  float** dataOutput = new float*[tb.plf_calls];
  float** checkOutput = new float*[tb.plf_calls];
  float** dataLeftInput = new float*[tb.plf_calls];
  float** dataRightInput = new float*[tb.plf_calls];
  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    dataOutput[i] = new float[tb.buffer_elements_out()];
    checkOutput[i] = new float[tb.buffer_elements_out()];
    dataLeftInput[i] = new float[tb.buffer_elements_left()];
    dataRightInput[i] = new float[tb.buffer_elements_right()];
    for (unsigned long int j = 0; j < 80; j++) {
      dataLeftInput[i][j] = (j%16)+1;
      dataRightInput[i][j] = (j%16)+1;
    }
    if (tb.combined_ev) {
      for (unsigned long int j = 0; j < tb.elements_per_plf(); j++) {
        dataLeftInput[i][80+j] = (j % 4) + 1;
        dataRightInput[i][80+j] = (j % 4) + 1;
      }
    } else {
      for (unsigned long int j = 0; j < tb.elements_per_plf(); j++) {
        dataLeftInput[i][80+j] = (j % 4) + 1;
        dataRightInput[i][64+j] = (j % 4) + 1;
      }
    }
  }


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
    out_buffer.read(dataOutput[i]);

    execution_ms.end[i] = t.elapsed();
  }
  xrt_profile.end();

  //Check///////////////////////////////////////////////////////////////////////////////////////////////////
  std::cout << "Data collected, checking for correctness ..." << std::endl;

  // Test correctness of return data
  std::string result = "Passed";
  unsigned int errors = 0;
  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    if (tb.combined_ev) {
      plf(&dataLeftInput[i][80], &dataRightInput[i][80], &checkOutput[i][0], &dataLeftInput[i][0], tb.alignment_sites, &dataLeftInput[i][16], &dataRightInput[i][16]);
    } else {
      plf(&dataLeftInput[i][80], &dataRightInput[i][64], &checkOutput[i][0], &dataLeftInput[i][0], tb.alignment_sites, &dataLeftInput[i][16], &dataRightInput[i][0]);
    }
    for (unsigned long int j = 0; j < tb.elements_per_plf(); j++) {
      if(dataOutput[i][j]!=checkOutput[i][j]) {
        std::cout << "Failed at block: " << i << ", index: " << j << ", " << dataOutput[i][j] << "!=" << checkOutput[i][j] << std::endl;
        errors++;
        result = " Failed with " + std::to_string(errors) + " errors";
        if (errors >= 20) {
          result = " Failed with more than 20 errors";
          break;
        }
      }
    }
  }

  //Result//////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << std::endl;
  std::cout << "Test result: " << result << std::endl;
  print_timing_data(execution_ms, (double)tb.data_size());
  //if (acap.get_target() == "hw") {
  //  std::string csvFile = std::string("data_hw_runs/") + acap.get_app_name() + "_" + acap.get_aie_name() + "_" + acap.get_pl_name() + "_freq" + STRINGIFY(PL_FREQ) + "_plfs" + STRINGIFY(tb.plf_calls) + ".csv";
  //  //write_to_csv(csvFile, execution_ms);
  //}

  //Cleanup////////////////////////////////////////////////////////////////////////////////////////////////

  for (unsigned int i = 0; i < tb.plf_calls; i++) {
    delete[] dataLeftInput[i];
    delete[] dataRightInput[i];
    delete[] dataOutput[i];
  }

  delete[] dataOutput;
  delete[] dataLeftInput;
  delete[] dataRightInput;
  dataOutput = nullptr;
  dataLeftInput = nullptr;
  dataRightInput = nullptr;

  delete[] mm2sleft_kernels;
  delete[] mm2sright_kernels;
  delete[] s2mm_kernels;
  delete[] mm2sleft_run;
  delete[] mm2sright_run;
  delete[] s2mm_run;
  mm2sleft_run = nullptr;
  mm2sright_run = nullptr;
  s2mm_run = nullptr;

  return 0;
}
