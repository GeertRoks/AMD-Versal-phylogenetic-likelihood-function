#define STRINGIFY_BACKEND(A) #A
#define STRINGIFY(A) STRINGIFY_BACKEND(A)

#include "include.h"
#include "timing.h"
#include "data.h"

#include <random>

int main(int argc, char* argv[]) {

  if(argc != 2)
    throw std::runtime_error(std::string("Not correct amount of parameters provided. Usage: ") + argv[0] + " </path/to/a.xclbin>\n");

  //acap_info acap(argv[1]);
  testbench_info tb;
  tb.alignment_sites=100;
  tb.plf_calls = 4;
  tb.window_size = 1024;
  tb.combined_ev = 0;
  tb.parallel_instances = 1;

  std::cout << std::left << std::endl;
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
  std::cout << "| instance out:           | " << std::setw(16) << tb.alignments_per_instance() << " | ";
  std::cout <<                                   std::setw(16) << tb.instance_elements_out()   << " | ";
  std::cout <<                                   std::setw(16) << tb.instance_size_out()       << " |" << std::endl;
  std::cout << "------------------------------------------------------------------------------------" << std::endl;
  std::cout << "| buffer left:            | " << std::setw(16) << tb.alignment_sites         << " | ";
  std::cout <<                                   std::setw(16) << tb.buffer_elements_left()  << " | ";
  std::cout <<                                   std::setw(16) << tb.buffer_size_left()      << " |" << std::endl;
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
  std::cout << std::right << std::endl;



  //Init//////////////////////////////////////////////////////////////////////////////////////////////////

  //device = xrt::device(0);
  xrt::device device = xrt::device("0000:5e:00.1");

  std::cout << "before bo" << std::endl;
  // Create Host <-> ACAP Memory buffers
  xrt::bo in_left_buffer = xrt::bo(device, tb.buffer_size_out(), xrt::bo::flags::normal, 0);
  std::cout << "between bo" << std::endl;
  xrt::bo out_buffer = xrt::bo(device, tb.buffer_size_out(), xrt::bo::flags::normal, 0);

  std::cout << "after bo" << std::endl;

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

  float** dataLeftInput = new float*[tb.plf_calls];
  float** dataOutput = new float*[tb.plf_calls];

  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    dataLeftInput[i] = new float[tb.buffer_elements_out()];
    dataOutput[i] = new float[tb.buffer_elements_out()];

    // load alignment data
    for (unsigned long int j = 0; j < tb.elements_per_plf(); j++) {
      dataLeftInput[i][j] = dis(gen);
    }

  }

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

    in_left_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    execution_ms.t1[i] = t.elapsed();

    // copy data
    //out_buffer.copy(in_left_buffer);

    execution_ms.t2[i] = t.elapsed();

    // Move data from ACAP Memory to host
    //out_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    //out_buffer.read(dataOutput[i]);
    in_left_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    in_left_buffer.read(dataOutput[i]);

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

  for (unsigned long int i = 0; i < tb.plf_calls; i++) {
    for (unsigned long int j = 0; j < tb.elements_per_plf(); j++) {
      if(dataOutput[i][j]!=dataLeftInput[i][j]) {
        std::cout << "Failed at block: " << i << ", index: " << j << ", " << dataOutput[i][j] << "!=" << dataLeftInput[i][j] << std::endl;
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
  print_timing_data(execution_ms, reference_ms, (double)tb.data_size(), tb.alignment_sites * tb.plf_calls, tb.plf_calls);


  //if (acap.get_target() == "hw") {
  //  std::string csvFile = std::string("data_hw_runs/") + acap.get_app_name() + "_" + acap.get_aie_name() + "_" + acap.get_pl_name() + "_freq" + STRINGIFY(PL_FREQ) + "_plfs" + STRINGIFY(tb.plf_calls) + ".csv";
  //  //write_to_csv(csvFile, execution_ms);
  //}

  //Cleanup////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "delete inner data arrays" << std::endl;
  for (unsigned int i = 0; i < tb.plf_calls; i++) {
    delete[] dataLeftInput[i];
    delete[] dataOutput[i];
  }

  std::cout << "delete data arrays" << std::endl;
  delete[] dataLeftInput;
  delete[] dataOutput;
  dataLeftInput = nullptr;
  dataOutput = nullptr;

  return 0;
}
