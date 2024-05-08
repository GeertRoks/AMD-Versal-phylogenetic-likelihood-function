
#include <adf.h>
#include "graph_window_PLF.h"

#define NUM_GRAPHS 1
#define LANES_PER_GRAPH 4
#define NUM_INPUTS (LANES_PER_GRAPH*NUM_GRAPHS)

#define WINDOW_DATA_SIZE 1024
#define WINDOW_BRANCH_SIZE 256
#define WINDOW_EV_SIZE 64

class PlioPLFGraph : public adf::graph {
public:
  adf::input_plio  plio_in_left_data[NUM_INPUTS];
  adf::input_plio  plio_in_left_branch[NUM_GRAPHS];
  adf::input_plio  plio_in_right_data[NUM_INPUTS];
  adf::input_plio  plio_in_right_branch[NUM_GRAPHS];
  adf::input_plio  plio_in_EV[NUM_GRAPHS];
  adf::input_plio  plio_in_alignments[NUM_GRAPHS];
  adf::output_plio plio_out[NUM_INPUTS];

  WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_EV_SIZE> graphs[NUM_GRAPHS] = {
    WindowPLFGraph<WINDOW_DATA_SIZE, WINDOW_BRANCH_SIZE, WINDOW_EV_SIZE>(20, 0)
  };

  PlioPLFGraph(){
    for(unsigned int i = 0; i < NUM_GRAPHS; i++) {
      plio_in_left_branch[i]  = adf::input_plio::create(plio_name_branch("in_branch", i, 0), adf::plio_128_bits, data_name("input", 1));
      plio_in_right_branch[i] = adf::input_plio::create(plio_name_branch("in_branch", i, 1), adf::plio_128_bits, data_name("input", 1));
      plio_in_EV[i]           = adf::input_plio::create(plio_name("in_EV", i),               adf::plio_128_bits, data_name("input", 1));
      plio_in_alignments[i]   = adf::input_plio::create(plio_name("in_alignments", i),       adf::plio_32_bits,  data_name("input", 2));

      adf::connect< adf::stream, adf::window<WINDOW_BRANCH_SIZE> >(plio_in_left_branch[i].out[0],  graphs[i].in_left_branch);
      adf::connect< adf::stream, adf::window<WINDOW_BRANCH_SIZE> >(plio_in_right_branch[i].out[0], graphs[i].in_right_branch);
      adf::connect< adf::stream, adf::window<WINDOW_EV_SIZE>     >(plio_in_EV[i].out[0],           graphs[i].in_EV);
      adf::connect< adf::stream >(plio_in_alignments[i].out[0], graphs[i].in_alignments);

      for(unsigned int j = 0; j < LANES_PER_GRAPH; j++) {
        const unsigned int idx = (i*LANES_PER_GRAPH)+j;

        plio_in_left_data[idx]  = adf::input_plio::create(plio_name_in("in", i, 0, j), adf::plio_128_bits, data_name("input", 0));
        plio_in_right_data[idx] = adf::input_plio::create(plio_name_in("in", i, 1, j), adf::plio_128_bits, data_name("input", 0));
        plio_out[idx]           = adf::output_plio::create(plio_name_out("out", i, j), adf::plio_128_bits, data_name("output", i , j));

        adf::connect< adf::stream, adf::window<WINDOW_DATA_SIZE> >(plio_in_left_data[idx].out[0],  graphs[i].in_left_data[j]);
        adf::connect< adf::stream, adf::window<WINDOW_DATA_SIZE> >(plio_in_right_data[idx].out[0], graphs[i].in_right_data[j]);
        adf::connect< adf::window<WINDOW_DATA_SIZE>, adf::stream >(graphs[i].out[j],             plio_out[idx].in[0]);
      }

    }
  };

private:
  std::string plio_name_branch(std::string str, unsigned int graph, unsigned int side) {
      std::ostringstream name;
      name << "plio_" << str << "_" << graph << "_" << side;
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
