#define STRINGIFY_BACKEND(A) #A
#define STRINGIFY(A) STRINGIFY_BACKEND(A)

#include "include.h"
#include "timing.h"
#include "data.h"

#include <random>

int main(int argc, char* argv[]) {

  if(argc != 2)
    throw std::runtime_error(std::string("Not correct amount of parameters provided. Usage: ") + argv[0] + " </path/to/a.xclbin>\n");

  acap_info acap(argv[1]);
  testbench_info tb;
  tb.alignment_sites=10000000;
  tb.plf_calls = 100;
  tb.window_size = 1024;
  tb.combined_ev = 1;
  tb.parallel_instances = 1;

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
  std::cout << "====================================================================================" << std::endl;
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
  }
  printf("\nconnected to datamover kernels\n");

  // Create the HLS kernel run handles and set the parameters
  xrt::run* mm2sleft_run = new xrt::run[tb.parallel_instances];
  xrt::run* mm2sright_run = new xrt::run[tb.parallel_instances];
  xrt::run* s2mm_run = new xrt::run[tb.parallel_instances];
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    mm2sleft_run[i] = xrt::run(mm2sleft_kernels[i]);
    mm2sleft_run[i].set_arg(0, tb.alignments_per_instance());
    mm2sleft_run[i].set_arg(1, tb.window_size);

    mm2sright_run[i] = xrt::run(mm2sright_kernels[i]);
    mm2sright_run[i].set_arg(0, tb.alignments_per_instance());
    mm2sright_run[i].set_arg(1, tb.window_size);

    s2mm_run[i] = xrt::run(s2mm_kernels[i]);
    s2mm_run[i].set_arg(0, tb.alignments_per_instance());
    s2mm_run[i].set_arg(1, tb.window_size);
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


  std::cout << "Start PLF calculation on accelerator ... " << std::endl;
  //t.reset();
  xrt_profile.start("roundtrip_exec_time");
  for (long int i = 0; i < tb.plf_calls; i++) {
    execution_ms.begin[i] = t.elapsed();

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

    execution_ms.end[i] = t.elapsed();
  }
  xrt_profile.end();


  //Result//////////////////////////////////////////////////////////////////////////////////////////////////

  timing_data reference_ms(tb.plf_calls);
  std::cout << std::endl;
  print_timing_data(execution_ms, reference_ms, (double) tb.data_size(), tb.alignment_sites * tb.plf_calls, tb.plf_calls);


  //if (acap.get_target() == "hw") {
  //  std::string csvFile = std::string("data_hw_runs/") + acap.get_app_name() + "_" + acap.get_aie_name() + "_" + acap.get_pl_name() + "_freq" + STRINGIFY(PL_FREQ) + "_plfs" + STRINGIFY(tb.plf_calls) + ".csv";
  //  //write_to_csv(csvFile, execution_ms);
  //}

  //Cleanup////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "delete handles" << std::endl;
  delete[] mm2sleft_kernels;
  delete[] mm2sright_kernels;
  delete[] s2mm_kernels;
  delete[] mm2sleft_run;
  delete[] mm2sright_run;
  delete[] s2mm_run;
  mm2sleft_kernels = nullptr;
  mm2sright_kernels = nullptr;
  s2mm_kernels = nullptr;
  mm2sleft_run = nullptr;
  mm2sright_run = nullptr;
  s2mm_run = nullptr;

  return 0;
}
