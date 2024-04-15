
#include <adf.h>
#include "kernels.h"

template<uint16_t window_size, uint16_t vector_size>
class WindowPassthroughGraph : public adf::graph {
public:
  adf::port<input>  data_in;
  adf::port<output> data_out;

  WindowPassthroughGraph(unsigned int col, unsigned int row) {
    k_passthrough = adf::kernel::create(window_passthrough_v<window_size,vector_size>);
    source(k_passthrough) = "kernels/window_passthrough.cpp";

    adf::connect<adf::window<window_size>>(data_in, k_passthrough.in[0]);
    adf::connect<adf::window<window_size>>(k_passthrough.out[0], data_out);

    adf::location<adf::kernel>(k_passthrough) = adf::tile(col, row);
    adf::location<adf::stack>(k_passthrough) = adf::bank(col, row, 2);
    adf::location<adf::buffer>(k_passthrough.in[0]) = adf::location<adf::kernel>(k_passthrough);
    if (row < 7) {
      adf::location<adf::buffer>(k_passthrough.out[0]) = {
        adf::address(col, row+1, 0x0),
        adf::address(col, row+1, 0x2000)
      };
    } else {
      adf::location<adf::buffer>(k_passthrough.out[0]) = {
        adf::address(col+1, row, 0x0),
        adf::address(col+1, row, 0x2000)
      };
    }

    adf::runtime<ratio>(k_passthrough) = 1;
  }

private:
  adf::kernel k_passthrough;
};
