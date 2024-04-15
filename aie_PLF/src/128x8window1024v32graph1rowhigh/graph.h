
#include <adf.h>
#include "graph_window_passthrough.h"

#define NUM_INPUTS 8
#define NUM_GRAPHS 1

#define WINDOW_SIZE 1024
#define VECTOR_SIZE 32

class PLIOPassthroughGraph : public adf::graph {
public:
  adf::input_plio  plio_in[NUM_INPUTS];
  adf::output_plio plio_out[NUM_INPUTS];

  WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE, NUM_INPUTS> graphs[NUM_GRAPHS] = {
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE, NUM_INPUTS>(20, 7)
  };

  PLIOPassthroughGraph(){
    for(unsigned int i = 0; i < NUM_INPUTS; i++) {

      plio_in[i] = adf::input_plio::create(plio_name("in", i), adf::plio_128_bits, data_name("input", 0));
      plio_out[i] = adf::output_plio::create(plio_name("out", i), adf::plio_128_bits, data_name("output", i));

      adf::connect<adf::stream, adf::window<WINDOW_SIZE>>(plio_in[i].out[0], graphs[0].data_in[i]);
      adf::connect<adf::window<WINDOW_SIZE>, adf::stream>(graphs[0].data_out[i], plio_out[i].in[0]);
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
