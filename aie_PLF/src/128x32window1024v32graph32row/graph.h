
#include <adf.h>
#include "graph_window_passthrough.h"

#define NUM_INPUTS 32
#define NUM_GRAPHS 32

#define WINDOW_SIZE 1024
#define VECTOR_SIZE 32

class PLIOPassthroughGraph : public adf::graph {
public:
  adf::input_plio  plio_in[NUM_INPUTS];
  adf::output_plio plio_out[NUM_INPUTS];

  WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE> graphs[NUM_GRAPHS] = {
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(7, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(8, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(9, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(10, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(11, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(12, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(13, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(14, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(15, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(16, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(17, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(18, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(19, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(20, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(21, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(22, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(23, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(24, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(25, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(26, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(27, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(28, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(29, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(30, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(31, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(32, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(33, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(34, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(35, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(36, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(37, 0),
    WindowPassthroughGraph<WINDOW_SIZE, VECTOR_SIZE>(38, 0),
  };

  PLIOPassthroughGraph(){
    for(unsigned int i = 0; i < NUM_INPUTS; i++) {

      plio_in[i] = adf::input_plio::create(plio_name("in", i), adf::plio_128_bits, data_name("input", 0));
      plio_out[i] = adf::output_plio::create(plio_name("out", i), adf::plio_128_bits, data_name("output", i));

      adf::connect<adf::stream, adf::window<WINDOW_SIZE>>(plio_in[i].out[0], graphs[i].data_in);
      adf::connect<adf::window<WINDOW_SIZE>, adf::stream>(graphs[i].data_out, plio_out[i].in[0]);
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
