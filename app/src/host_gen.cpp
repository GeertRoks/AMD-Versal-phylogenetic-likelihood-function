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
  xrt::kernel mm2sleft_kernels[tb.parallel_instances];
  xrt::kernel mm2sright_kernels[tb.parallel_instances];
  xrt::kernel s2mm_kernels[tb.parallel_instances];
  for (unsigned int i = 0; i < tb.parallel_instances; i++) {
    mm2sleft_kernels[i] = xrt::kernel(*acap.get_device(), *acap.get_uuid(), kernel_name("mm2sleft", i));
    mm2sright_kernels[i] = xrt::kernel(*acap.get_device(), *acap.get_uuid(), kernel_name("mm2sright", i));
    s2mm_kernels[i] = xrt::kernel(*acap.get_device(), *acap.get_uuid(), kernel_name("s2mm", i));
  }
  printf("\nconnected to datamover kernels\n");

  // Create the HLS kernel run handles and set the parameters
  xrt::run mm2sleft_run[tb.parallel_instances];
  xrt::run mm2sright_run[tb.parallel_instances];
  xrt::run s2mm_run[tb.parallel_instances];
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
  std::cout << "Setup data gathering structures ... " << std::endl << std::endl;
  xrt::queue main_queue[tb.parallel_instances];
  xrt::queue right_queue[tb.parallel_instances];
  xrt::queue output_queue[tb.parallel_instances];
  xrt::queue::event right_done[tb.parallel_instances];
  xrt::queue::event out_done[tb.parallel_instances];
  xrt::queue::event instance_done[tb.parallel_instances];

  auto xrt_profile = xrt::profile::user_range("roundtrip_exec_time", "The execution time of the full test");
  Timer t;
  timing_data execution_ms;
  execution_ms.init(tb.plf_calls);


  std::cout << "Start PLF calculation on accelerator ... " << std::endl;
  //t.reset();
  xrt_profile.start("roundtrip_exec_time");
  for (long int i = 0; i < tb.plf_calls; i++) {
    execution_ms.begin[i] = t.elapsed();

    execution_ms.t1[i] = t.elapsed();

    // Execute PLF calculation parallelized on Versal
    for (unsigned int k = 0; k < tb.parallel_instances; k++) {

      // execute acceleration platform
      main_queue[k].enqueue([&mm2sleft_run, k] {mm2sleft_run[k].start(); mm2sleft_run[k].wait();});
      right_done[k] = right_queue[k].enqueue([&mm2sright_run, k] {mm2sright_run[k].start(); mm2sright_run[k].wait();});
      out_done[k] = output_queue[k].enqueue([&s2mm_run, k] {s2mm_run[k].start(); s2mm_run[k].wait();});
      main_queue[k].enqueue(right_done[k]);
      instance_done[k] = main_queue[k].enqueue(out_done[k]);

    }

    // Sync all threads
    for (unsigned int k = 0; k < tb.parallel_instances; k++) {
      instance_done[k].wait();
    }

    execution_ms.t2[i] = t.elapsed();

    execution_ms.end[i] = t.elapsed();
  }
  xrt_profile.end();


  //Result//////////////////////////////////////////////////////////////////////////////////////////////////

  timing_data reference_ms;
  reference_ms.init(tb.plf_calls);
  std::cout << std::endl;
  print_timing_data(execution_ms, reference_ms, (double) tb.data_size(), tb.alignment_sites * tb.plf_calls, tb.plf_calls);


  if (acap.get_target() == "hw") {
    std::string csvFile = std::string("data_hw_pl_generator_runs/") + acap.get_app_name() + "_" + acap.get_aie_name() + "_" + acap.get_pl_name() + "_plfs" + std::to_string(tb.plf_calls) + "_alignments" + std::to_string(tb.alignment_sites) + "_usedgraphs" + std::to_string(tb.parallel_instances) + ".csv";
    write_to_csv(csvFile, execution_ms);
  }

  //Cleanup////////////////////////////////////////////////////////////////////////////////////////////////

  execution_ms.destroy();
  reference_ms.destroy();

  std::cout << "delete handles" << std::endl;
  //delete[] mm2sleft_kernels;
  //delete[] mm2sright_kernels;
  //delete[] s2mm_kernels;
  //delete[] mm2sleft_run;
  //delete[] mm2sright_run;
  //delete[] s2mm_run;
  //mm2sleft_kernels = nullptr;
  //mm2sright_kernels = nullptr;
  //s2mm_kernels = nullptr;
  //mm2sleft_run = nullptr;
  //mm2sright_run = nullptr;
  //s2mm_run = nullptr;

  return 0;
}
