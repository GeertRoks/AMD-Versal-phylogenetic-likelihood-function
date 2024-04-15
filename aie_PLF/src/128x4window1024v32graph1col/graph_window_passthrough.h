
#include <adf.h>
#include "kernels.h"

template<uint16_t window_size, uint16_t vector_size, uint16_t num_inputs>
class WindowPassthroughGraph : public adf::graph {
public:
  adf::port<input>  data_in[num_inputs];
  adf::port<output> data_out[num_inputs];

  WindowPassthroughGraph(unsigned int start_col, unsigned int start_row) {
    for (unsigned int i = 0; i < num_inputs; i++) {
      k_passthrough[i] = adf::kernel::create(window_passthrough_v<window_size,vector_size>);
      source(k_passthrough[i]) = "kernels/window_passthrough.cpp";

      adf::connect<adf::window<window_size>>(data_in[i], k_passthrough[i].in[0]);
      adf::connect<adf::window<window_size>>(k_passthrough[i].out[0], data_out[i]);

      adf::location<adf::kernel>(k_passthrough[i]) = adf::tile(start_col, start_row+i);
      adf::location<adf::stack>(k_passthrough[i]) = adf::bank(start_col, start_row+i, 2);
      adf::location<adf::buffer>(k_passthrough[i].in[0]) = adf::location<adf::kernel>(k_passthrough[i]);
      adf::location<adf::buffer>(k_passthrough[i].out[0]) = {
        adf::address(start_col, start_row+i+1, 0x0),
        adf::address(start_col, start_row+i+1, 0x2000)
      };

      adf::runtime<ratio>(k_passthrough[i]) = 1;
    }
  }

private:
  adf::kernel k_passthrough[num_inputs];
};
