
#include <adf.h>
#include "graph_window_PLF.h"

#define NUM_GRAPHS 4
#define LANES_PER_GRAPH 4
#define NUM_INPUTS (LANES_PER_GRAPH*NUM_GRAPHS)

#define WINDOW_DATA_SIZE 8192 // (8192/sizeof(float)) / 4 values per alignment per lane = (8192/4)/4 = 2048/4 = 512 alignments per window
#define WINDOW_BRANCH_SIZE 64 // 16 values per matrix * sizeof(float) = 16*4 = 64 bytes per window
#define WINDOW_HALF_EV_SIZE 32 // 8 values per matrix * sizeof(float) = 8*4 = 32 bytes per window (half of EV matrix)

class PlioPLFGraph : public adf::graph {
public:
  adf::input_plio  plio_in_left_data[NUM_INPUTS];
  adf::input_plio  plio_in_right_data[NUM_INPUTS];
  adf::output_plio plio_out[NUM_INPUTS];

  WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_HALF_EV_SIZE> graphs[NUM_GRAPHS] = {
    WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_HALF_EV_SIZE>(20, 0),
    WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_HALF_EV_SIZE>(20, 0),
    WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_HALF_EV_SIZE>(20, 0),
    WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_HALF_EV_SIZE>(20, 0)
  };

  PlioPLFGraph(){
    for(unsigned int i = 0; i < NUM_GRAPHS; i++) {

      for(unsigned int j = 0; j < LANES_PER_GRAPH; j++) {
        const unsigned int idx = (i*LANES_PER_GRAPH)+j;

        // data input
        plio_in_left_data[idx]  = adf::input_plio::create(plio_name_in("in", i, 0, j), adf::plio_128_bits, data_name("inputcombinedevleft", j));
        plio_in_right_data[idx] = adf::input_plio::create(plio_name_in("in", i, 1, j), adf::plio_128_bits, data_name("inputcombinedevright", j));
        adf::connect< adf::stream, adf::window<WINDOW_DATA_SIZE+WINDOW_BRANCH_SIZE+WINDOW_HALF_EV_SIZE> >(plio_in_left_data[idx].out[0],  graphs[i].in_left_data[j]);
        adf::connect< adf::stream, adf::window<WINDOW_DATA_SIZE+WINDOW_BRANCH_SIZE+WINDOW_HALF_EV_SIZE> >(plio_in_right_data[idx].out[0], graphs[i].in_right_data[j]);

        // output
        plio_out[idx] = adf::output_plio::create(plio_name_out("out", i, j), adf::plio_128_bits, data_name("output", i , j));
        adf::connect< adf::window<WINDOW_DATA_SIZE>, adf::stream >(graphs[i].out[j], plio_out[idx].in[0]);

      }

    }
  };

private:
  std::string plio_name_branch(std::string str, unsigned int graph, unsigned int side, unsigned int input) {
      std::ostringstream name;
      name << "plio_" << str << "_" << graph << "_" << side << "_" << input;
      return name.str();
  }
  std::string plio_name_in(std::string str, unsigned int graph, unsigned int side, unsigned int input) {
      std::ostringstream name;
      name << "plio_" << str << "_" << graph << "_" << side << "_" << input;
      return name.str();
  }
  std::string plio_name_out(std::string str, unsigned int graph, unsigned int output) {
      std::ostringstream name;
      name << "plio_" << str << "_" << graph << "_" << output;
      return name.str();
  }
  std::string plio_name(std::string str, unsigned int it) {
      std::ostringstream name;
      name << "plio_" << str << "_" << it;
      return name.str();
  }
  std::string data_name(std::string str, unsigned int it) {
      std::ostringstream name;
      name << "data/" << str << it << ".txt";
      return name.str();
  }
  std::string data_name(std::string str, unsigned int graph, unsigned int inout) {
      std::ostringstream name;
      name << "data/" << str << graph << "_" << inout << ".txt";
      return name.str();
  }
};
