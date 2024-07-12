
#include <adf.h>
#include "graph_stream_PLF.h"

#define NUM_GRAPHS 4
#define LANES_PER_GRAPH 4
#define NUM_INPUTS (LANES_PER_GRAPH*NUM_GRAPHS)

class PlioPLFGraph : public adf::graph {
public:
  adf::input_plio  plio_in_left_data[NUM_INPUTS];
  adf::input_plio  plio_in_right_data[NUM_INPUTS];
  adf::output_plio plio_out[NUM_INPUTS];

  StreamPLFGraph graphs[NUM_GRAPHS] = {
    StreamPLFGraph(20, 0),
    StreamPLFGraph(20, 0),
    StreamPLFGraph(20, 0),
    StreamPLFGraph(20, 0)
  };

  PlioPLFGraph(){
    for(unsigned int i = 0; i < NUM_GRAPHS; i++) {

      for(unsigned int j = 0; j < LANES_PER_GRAPH; j++) {
        const unsigned int idx = (i*LANES_PER_GRAPH)+j;

        // data input
        plio_in_left_data[idx]  = adf::input_plio::create(plio_name_in("in", i, 0, j), adf::plio_128_bits, data_name("stream/inputcombinedevleft", j));
        plio_in_right_data[idx] = adf::input_plio::create(plio_name_in("in", i, 1, j), adf::plio_128_bits, data_name("stream/inputcombinedevright", j));
        adf::connect< adf::stream >(plio_in_left_data[idx].out[0],  graphs[i].in_left_data[j]);
        adf::connect< adf::stream >(plio_in_right_data[idx].out[0], graphs[i].in_right_data[j]);

        // output
        plio_out[idx] = adf::output_plio::create(plio_name_out("out", i, j), adf::plio_128_bits, data_name("output", i , j));
        adf::connect< adf::stream >(graphs[i].out[j], plio_out[idx].in[0]);

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
