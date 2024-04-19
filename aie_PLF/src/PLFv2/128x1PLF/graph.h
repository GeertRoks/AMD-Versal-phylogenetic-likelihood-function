
#include <adf.h>
#include "graph_window_PLF.h"

#define NUM_GRAPHS 1
#define LANES_PER_GRAPH 4
#define NUM_INPUTS (LANES_PER_GRAPH*NUM_GRAPHS)

#define WINDOW_DATA_SIZE 16
#define WINDOW_BRANCH_SIZE 256
#define WINDOW_EV_SIZE 64

class PlioPLFGraph : public adf::graph {
public:
  adf::input_plio  plio_in_left_data[NUM_INPUTS];
  adf::input_plio  plio_in_left_branch[NUM_INPUTS];
  adf::input_plio  plio_in_right_data[NUM_INPUTS];
  adf::input_plio  plio_in_right_branch[NUM_INPUTS];
  adf::input_plio  plio_in_EV[NUM_INPUTS];
  adf::output_plio plio_out[NUM_INPUTS];

  WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_EV_SIZE> graphs[NUM_GRAPHS] = {
    WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_EV_SIZE>(20, 0)
  };

  PlioPLFGraph(){
    for(unsigned int i = 0; i < NUM_GRAPHS; i++) {
      for(unsigned int j = 0; j < LANES_PER_GRAPH; j++) {
        const unsigned int idx = (i*LANES_PER_GRAPH)+j;

        plio_in_left_data[idx]    = adf::input_plio::create(plio_name("in_left_data", idx), adf::plio_128_bits, data_name("input", 0));
        plio_in_left_branch[idx]  = adf::input_plio::create(plio_name("in_left_branch", idx), adf::plio_128_bits, data_name("input", 1));
        plio_in_right_data[idx]   = adf::input_plio::create(plio_name("in_right_data", idx), adf::plio_128_bits, data_name("input", 0));
        plio_in_right_branch[idx] = adf::input_plio::create(plio_name("in_right_branch", idx), adf::plio_128_bits, data_name("input", 1));

        plio_in_EV[idx] = adf::input_plio::create(plio_name("in_EV", idx), adf::plio_128_bits, data_name("input", 1));

        plio_out[idx] = adf::output_plio::create(plio_name("out", idx), adf::plio_128_bits, data_name("output", idx));

        adf::connect< adf::stream, adf::window<WINDOW_DATA_SIZE>   >(plio_in_left_data[idx].out[0],    graphs[0].data_in_left_data[idx]);
        adf::connect< adf::stream, adf::window<WINDOW_BRANCH_SIZE> >(plio_in_left_branch[idx].out[0],  graphs[0].data_in_left_branch[idx]);
        adf::connect< adf::stream, adf::window<WINDOW_DATA_SIZE>   >(plio_in_right_data[idx].out[0],   graphs[0].data_in_right_data[idx]);
        adf::connect< adf::stream, adf::window<WINDOW_BRANCH_SIZE> >(plio_in_right_branch[idx].out[0], graphs[0].data_in_right_branch[idx]);

        adf::connect< adf::stream, adf::window<WINDOW_EV_SIZE>     >(plio_in_EV[idx].out[0],           graphs[0].data_in_EV[idx]);

        adf::connect< adf::window<WINDOW_DATA_SIZE>, adf::stream   >(graphs[0].data_out[idx],          plio_out[idx].in[0]);
      }
    }
  };
  std::string plio_name(std::string dir, unsigned int it) {
      std::ostringstream name;
      name << "plio_" << dir << it;
      return name.str();
  }
  std::string data_name(std::string dir, unsigned int it) {
      std::ostringstream name;
      name << "data/" << dir << it << ".txt";
      return name.str();
  }
};
