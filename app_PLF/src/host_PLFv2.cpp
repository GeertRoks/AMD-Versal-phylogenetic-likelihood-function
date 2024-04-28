
#ifndef BLOCKS // number of round trips
#define BLOCKS 1
#endif

#define STRINGIFY_BACKEND(A) #A
#define STRINGIFY(A) STRINGIFY_BACKEND(A)

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <random>
#include <cstdlib>
#include <regex>
#include <cassert>

#include "xrt/xrt_device.h"
#include "experimental/xrt_xclbin.h"
#include "xrt/xrt_kernel.h"
#include "xrt/xrt_graph.h"
#include "xrt/xrt_bo.h"
#include "experimental/xrt_profile.h"

#include "timing.h"
#include "plf.h"
#include "data.h"

std::string kernel_name(std::string kernel, unsigned int it) {
  std::ostringstream name;
  name << kernel << ":{" << kernel << "_" << it << "}";
  return name.str();
}

std::string splitFilename(const char* path, size_t index) {
    std::string filename = path;
    std::vector<std::string> parts;
    size_t pos = 0;
    std::string delimiter = "_";

    // Find the last occurrence of the delimiter to get the filename
    size_t lastSlashPos = filename.find_last_of("/\\");
    if (lastSlashPos != std::string::npos) {
        filename = filename.substr(lastSlashPos + 1);
    }

    // Split the filename into parts
    while ((pos = filename.find(delimiter)) != std::string::npos) {
        parts.push_back(filename.substr(0, pos));
        filename.erase(0, pos + delimiter.length());
    }
    parts.push_back(filename); // Add the last part

    // Check if the index is within the bounds of the parts vector
    if (index >= 0 && index < parts.size()) {
        return parts[index];
    } else {
        // Return an empty string if the index is out of bounds
        return "";
    }
}


unsigned int findNumAieIOInXclbin(const std::string& input) {
    size_t pos = input.find('x');
    if (pos == std::string::npos) {
        // 'x' not found, return 0 or handle as needed
        return 0;
    }
    std::string valueStr = input.substr(pos + 1);
    unsigned int value = std::stoi(valueStr);
    return value;
}

unsigned int findKernelElementsInXclbin(std::string path) {
  std::regex pattern("v(\\d+)");
  std::smatch matches;
  if (std::regex_search(path, matches, pattern) && matches.size() > 1) {
    std::string value = matches[1].str(); // The first capture group contains the number
    unsigned long lresult = std::stoul(value, nullptr, 10);
    if (lresult > std::numeric_limits<unsigned int>::max()) {
        throw std::out_of_range("Value is too large for unsigned int");
    }
    return static_cast<unsigned int>(lresult);
  } else {
    std::cout << "Pattern not found." << std::endl;
    return 0;
  }
}

int main(int argc, char* argv[]) {

  if(argc != 4)
    throw std::runtime_error(std::string("Not enough parameters provided. Usage: ") + argv[0] + " /path/to/a.xclbin buffer_size chunk_size\n");

  char* xclbinFile=argv[1];
  const unsigned int BUFFER_SIZE = atoi(argv[2]);
  const unsigned int CHUNK_SIZE = atoi(argv[3]);

  const std::string APP_NAME = splitFilename(xclbinFile, 0);
  const std::string PL_NAME = splitFilename(xclbinFile, 1);
  const std::string AIE_NAME = splitFilename(xclbinFile, 2);

  //const unsigned int KERNEL_ELEMENTS = findKernelElementsInXclbin(xclbinFile);
  //if (!KERNEL_ELEMENTS) {
  //  throw std::runtime_error(std::string("KERNEL_ELEMENTS value not found\n"));
  //}
  const unsigned int KERNEL_ELEMENTS = 16;
  const unsigned int NUM_AIE_IO = findNumAieIOInXclbin(AIE_NAME);
  if (!NUM_AIE_IO) {
    throw std::runtime_error(std::string("NUM_AIE_IO value not found\n"));
  }
  std::cout << "NUM_AIE_IO " << NUM_AIE_IO << std::endl;

  const unsigned int CHUNKS = BUFFER_SIZE/CHUNK_SIZE;
  const unsigned int DATA_SIZE = BUFFER_SIZE * BLOCKS;

  const unsigned int WORD_SIZE = sizeof(float);
  const unsigned int DATA_ELEMENTS = DATA_SIZE/WORD_SIZE;
  const unsigned int BUFFER_ELEMENTS = BUFFER_SIZE/WORD_SIZE;
  const unsigned int CHUNK_ELEMENTS = CHUNK_SIZE/WORD_SIZE;
  const unsigned int AIE_OPS = BUFFER_ELEMENTS/KERNEL_ELEMENTS;

  assert(CHUNKS % NUM_AIE_IO == 0);

  std::cout << std::endl;
  std::cout << "=======================================================================" << std::endl;
  std::cout << "| test name:           | " << APP_NAME << std::endl;
  std::cout << "| PL name:             | " << PL_NAME << std::endl;
  std::cout << "| AIE name:            | " << AIE_NAME << std::endl;
  std::cout << "=======================================================================" << std::endl;
  std::cout << "| data size:           | " << DATA_SIZE << std::endl;
  std::cout << "| data elements:       | " << DATA_ELEMENTS << std::endl;
  std::cout << "| word size:           | " << WORD_SIZE << std::endl;
  std::cout << "-----------------------------------------------------------------------" << std::endl;
  std::cout << "| buffer size:         | " << BUFFER_SIZE << std::endl;
  std::cout << "| buffer elements:     | " << BUFFER_ELEMENTS << std::endl;
  std::cout << "| blocks:              | " << BLOCKS << std::endl;
  std::cout << "-----------------------------------------------------------------------" << std::endl;
  std::cout << "| chunk size:          | " << CHUNK_SIZE << std::endl;
  std::cout << "| chunk elements:      | " << CHUNK_ELEMENTS << std::endl;
  std::cout << "| chunks:              | " << CHUNKS << std::endl;
  std::cout << "-----------------------------------------------------------------------" << std::endl;
  std::cout << "| num aie io:          | " << NUM_AIE_IO << std::endl;
  std::cout << "| aie ops per buffer:  | " << AIE_OPS << std::endl;
  std::cout << "| aie kernel elements: | " << KERNEL_ELEMENTS << std::endl;
  std::cout << "=======================================================================" << std::endl;


  //XRTsetup//////////////////////////////////////////////////////////////////////////////////////////////

  auto xclbin = xrt::xclbin(xclbinFile);

  std::string target;
  switch(xclbin.get_target_type()) {
    case xrt::xclbin::target_type::sw_emu:
      target = "sw_emu";
      break;
    case xrt::xclbin::target_type::hw_emu:
      target = "hw_emu";
      break;
    case xrt::xclbin::target_type::hw:
      target = "hw";
      break;
    default:
      throw std::runtime_error("Invalid target");
  }

  xrt::device device;
  if (target == "sw_emu" || target == "hw_emu") {
    device = xrt::device(0);
  } else if (target == "hw") {
    device = xrt::device("0000:5e:00.1");
  }
  if(device == nullptr) {
    throw std::runtime_error("No valid device handle found. Run `xbutil examine` and look for the correct BDF.\n If xbutil is not found, then source the xrt setup file: `source /opt/xilinx/xrt/setup.sh`\n");
  }


  std::cout << "| device name:         | " << device.get_info<xrt::info::device::name>() << std::endl;
  std::cout << "| device bdf:          | " << device.get_info<xrt::info::device::bdf>() << std::endl;
  std::cout << "| xclbin host_type:    | " << target << std::endl;
  //std::cout << "| xclbin uuid:         | " << xclbin.get_uuid().to_string() << std::endl;
  std::cout << "=======================================================================" << std::endl;
  std::cout << std::endl;

  auto kernels = xclbin.get_kernels();
  for (auto kernel : kernels) {
    std::cout << "kernel: " << kernel.get_name() << std::endl;
  }
  std::cout << std::endl;

  auto xclbin_uuid = device.load_xclbin(xclbin);

  //Init//////////////////////////////////////////////////////////////////////////////////////////////////

  // Create HLS kernel handles
  xrt::kernel* mm2sleft_kernels = new xrt::kernel[NUM_AIE_IO];
  xrt::kernel* mm2sright_kernels = new xrt::kernel[NUM_AIE_IO];
  xrt::kernel* s2mm_kernels = new xrt::kernel[NUM_AIE_IO];
  for (unsigned int i = 0; i < NUM_AIE_IO; i++) {
    mm2sleft_kernels[i] = xrt::kernel(device, xclbin_uuid, kernel_name("mm2sleft", i));
    mm2sright_kernels[i] = xrt::kernel(device, xclbin_uuid, kernel_name("mm2sright", i));
    s2mm_kernels[i] = xrt::kernel(device, xclbin_uuid, kernel_name("s2mm", i));
    printf("mm2sleft[%i] mem group: %i, ", i, mm2sleft_kernels[i].group_id(0));
    printf("mm2sright[%i] mem group: %i, ", i, mm2sright_kernels[i].group_id(0));
    printf("s2mm[%i] mem group: %i\n", i, s2mm_kernels[i].group_id(0));
  }
  printf("connected to datamover kernels\n");

  // Create Host <-> ACAP Memory buffers
  xrt::bo in_left_buffer = xrt::bo(device, BUFFER_SIZE+(80*4), xrt::bo::flags::normal, mm2sleft_kernels[0].group_id(0));
  xrt::bo in_right_buffer = xrt::bo(device, BUFFER_SIZE+(64*4), xrt::bo::flags::normal, mm2sright_kernels[0].group_id(0));
  xrt::bo out_buffer = xrt::bo(device, BUFFER_SIZE, xrt::bo::flags::normal, s2mm_kernels[0].group_id(0));

  xrt::bo* in_left_chunks = new xrt::bo[CHUNKS];
  xrt::bo* in_right_chunks = new xrt::bo[CHUNKS];
  xrt::bo* out_chunks = new xrt::bo[CHUNKS];
  for (unsigned int i = 0; i < CHUNKS; i++) {
    in_left_chunks[i] = xrt::bo(in_left_buffer, CHUNK_SIZE, (i*CHUNK_SIZE));
    in_right_chunks[i] = xrt::bo(in_right_buffer, CHUNK_SIZE, (i*CHUNK_SIZE));
    out_chunks[i] = xrt::bo(out_buffer, CHUNK_SIZE, (i*CHUNK_SIZE));
  }

  // Create the HLS kernel run handles and set the parameters
  xrt::run* mm2sleft_run = new xrt::run[NUM_AIE_IO];
  xrt::run* mm2sright_run = new xrt::run[NUM_AIE_IO];
  xrt::run* s2mm_run = new xrt::run[NUM_AIE_IO];
  for (unsigned int i = 0; i < NUM_AIE_IO; i++) {
    mm2sleft_run[i] = xrt::run(mm2sleft_kernels[i]);
    mm2sleft_run[i].set_arg(1, CHUNK_ELEMENTS);
    mm2sright_run[i] = xrt::run(mm2sright_kernels[i]);
    mm2sright_run[i].set_arg(1, CHUNK_ELEMENTS);
    s2mm_run[i] = xrt::run(s2mm_kernels[i]);
    s2mm_run[i].set_arg(1, CHUNK_ELEMENTS);
  }

  // Create the AI engine graph handle
  if (target == "sw_emu") {
    // In sw_emu the graph needs to be started manually
    std::cout << "enable graph" << std::endl;
    xrt::graph aie_graph(device, xclbin_uuid, "mygraph");
    aie_graph.run(AIE_OPS*BLOCKS);
    //unsigned int aie_kernel_iterations = AIE_OPS;
    // enable if using async RTP
    //aie_graph.update("mygraph.graph.param_in_iterations", aie_kernel_iterations);
    // sync RTP uses the update call as a trigger for kernel execution (UG1079 - Run-Time Graph Control API) }
  }

  //Check before run///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(NO_PRERUN_CHECK) || NO_PRERUN_CHECK == 0

  char user_response;
  bool validInput = false;
  bool ready_for_run = false;

  while (!validInput) {
    std::cout << std::endl;
    std::cout << "Ready to continue with test? [Y/n]: ";
    std::cin >> user_response;

    user_response = tolower(user_response);

    if (user_response == 'y' || user_response == 'n' || std::cin.fail()) {
      validInput = true;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      if (user_response == 'y' || std::cin.fail()) {
        ready_for_run = true;
      }
    } else {
      std::cout << "You may only type 'y' or 'n'.\n";
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }

  if (!ready_for_run) {
    return 0;
  }
  std::cout << std::endl;

#endif

  //Run///////////////////////////////////////////////////////////////////////////////////////////////////

  auto xrt_profile = xrt::profile::user_range("roundtrip_exec_time", "The execution time of the full test");

  Timer t;
  timing_data<BLOCKS> execution_ms;

  float** dataOutput = new float*[BLOCKS];
  float** checkOutput = new float*[BLOCKS];
  float** dataLeftInput = new float*[BLOCKS];
  float** dataRightInput = new float*[BLOCKS];
  for (unsigned long int i = 0; i < BLOCKS; i++) {
    dataOutput[i] = new float[BUFFER_ELEMENTS];
    checkOutput[i] = new float[BUFFER_ELEMENTS];
    dataLeftInput[i] = new float[80+BUFFER_ELEMENTS];
    dataRightInput[i] = new float[64+BUFFER_ELEMENTS];
    for (unsigned long int j = 0; j < 80; j++) {
      dataLeftInput[i][j] = (j%16)+1;
      dataRightInput[i][j] = (j%16)+1;
    }
    for (unsigned long int j = 0; j < BUFFER_ELEMENTS; j++) {
      dataLeftInput[i][80+j] = (j % 4) + 1;
      dataRightInput[i][64+j] = (j % 4) + 1;
    }
  }


  t.reset();
  xrt_profile.start("roundtrip_exec_time");
  for (long int i = 0; i < BLOCKS; i++) {
    execution_ms.begin[i] = t.elapsed();

    // Move data from host to ACAP Memory
    in_left_buffer.write(dataLeftInput[i]);
    in_right_buffer.write(dataRightInput[i]);

    in_left_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    in_right_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    execution_ms.t1[i] = t.elapsed();

    for (long int j = 0; j < (CHUNKS/NUM_AIE_IO); j++) {
      for (unsigned int k = 0; k < NUM_AIE_IO; k++) {
        mm2sleft_run[k].set_arg(0, in_left_chunks[j*NUM_AIE_IO+k]);
        mm2sright_run[k].set_arg(0, in_right_chunks[j*NUM_AIE_IO+k]);
        s2mm_run[k].set_arg(0, out_chunks[j*NUM_AIE_IO+k]);
        s2mm_run[k].start();
        mm2sleft_run[k].start();
        mm2sright_run[k].start();
      }
      for (unsigned int k = 0; k < NUM_AIE_IO; k++) {
        mm2sleft_run[k].wait();
        mm2sright_run[k].wait();
        s2mm_run[k].wait();
      }
    }

    execution_ms.t2[i] = t.elapsed();

    // Move data from ACAP Memory to host
    out_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    out_buffer.read(dataOutput[i]);

    execution_ms.end[i] = t.elapsed();
  }
  xrt_profile.end();

  //Check///////////////////////////////////////////////////////////////////////////////////////////////////

  // Test correctness of return data
  std::string result = "Passed";
  unsigned int errors = 0;
  for (unsigned long int i = 0; i < BLOCKS; i++) {
    plf(&dataLeftInput[i][80], &dataRightInput[i][64], &checkOutput[i][0], &dataLeftInput[i][64], BUFFER_ELEMENTS/16, &dataLeftInput[i][0], &dataRightInput[i][0]);
    for (unsigned long int j = 0; j < BUFFER_ELEMENTS; j++) {
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
  //std::cout << "Test result: " << result << std::endl;
  print_timing_data(execution_ms, (double)DATA_SIZE);
  if (target == "hw") {
    std::string csvFile = std::string("data_hw_runs/") + STRINGIFY(AIE) + "_" + STRINGIFY(PL) + "_freq" + STRINGIFY(PL_FREQ) + "__buf" + std::to_string(BUFFER_SIZE) + "_chunk" + std::to_string(CHUNK_SIZE) + "_blocks" + STRINGIFY(BLOCKS) + ".csv";
    write_to_csv(csvFile ,execution_ms);
  }

  //Cleanup////////////////////////////////////////////////////////////////////////////////////////////////

  for (int i = 0; i < BLOCKS; i++) {
    delete[] dataLeftInput[i];
    delete[] dataRightInput[i];
    delete[] dataOutput[i];
  }

  delete[] dataOutput;
  delete[] dataLeftInput;
  delete[] dataRightInput;
  delete[] in_left_chunks;
  delete[] in_right_chunks;
  delete[] out_chunks;
  dataOutput = nullptr;
  dataLeftInput = nullptr;
  dataRightInput = nullptr;
  in_left_chunks = nullptr;
  in_right_chunks = nullptr;
  out_chunks = nullptr;

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
