
#include <adf.h>
#include "graph_window_PLF.h"

#define NUM_INPUTS 1
#define NUM_GRAPHS 1

#define WINDOW_DATA_SIZE 256
#define WINDOW_BRANCH_SIZE 1024

class PlioPLFGraph : public adf::graph {
public:
  adf::input_plio  plio_in0[NUM_INPUTS];
  adf::input_plio  plio_in1[NUM_INPUTS];
  adf::output_plio plio_out[NUM_INPUTS];

  WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, NUM_INPUTS> graphs[NUM_GRAPHS] = {
    WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, NUM_INPUTS>(20, 0)
  };

  PlioPLFGraph(){
    for(unsigned int i = 0; i < NUM_INPUTS; i++) {

      plio_in0[i] = adf::input_plio::create(plio_name("in1", i), adf::plio_128_bits, data_name("input", 0));
      plio_in1[i] = adf::input_plio::create(plio_name("in2", i), adf::plio_128_bits, data_name("input", 1));
      plio_out[i] = adf::output_plio::create(plio_name("out", i), adf::plio_128_bits, data_name("output", i));

      adf::connect<adf::stream, adf::window<WINDOW_DATA_SIZE>>(plio_in0[i].out[0], graphs[0].data_in0[i]);
      adf::connect<adf::stream, adf::window<WINDOW_BRANCH_SIZE>>(plio_in1[i].out[0], graphs[0].data_in1[i]);
      adf::connect<adf::window<WINDOW_DATA_SIZE>, adf::stream>(graphs[0].data_out[i], plio_out[i].in[0]);
    }
  };
  std::string plio_name(std::string dir, unsigned int it) {
      std::ostringstream name;
      name << "plio_data_" << dir << it;
      return name.str();
  }
  std::string data_name(std::string dir, unsigned int it) {
      std::ostringstream name;
      name << "data/" << dir << it << ".txt";
      return name.str();
  }
};
