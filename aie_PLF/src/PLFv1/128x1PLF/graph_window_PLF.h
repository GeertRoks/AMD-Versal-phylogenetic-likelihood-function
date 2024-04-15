
#include <adf.h>
#include "kernels.h"

template<uint16_t window_data_size, uint16_t window_branch_size, uint16_t num_inputs>
class WindowPLFGraph : public adf::graph {
public:
  adf::port<input>  data_in0[num_inputs];
  adf::port<input>  data_in1[num_inputs];
  adf::port<output> data_out[num_inputs];

  WindowPLFGraph(unsigned int start_col, unsigned int start_row) {
    for (unsigned int i = 0; i < num_inputs; i++) {
      k_mmul_left[i] = adf::kernel::create(mmul_branch<window_data_size>);
      source(k_mmul_left[i]) = "kernels/mmul_branch.cpp";

      adf::connect<adf::window<window_data_size>>(data_in0[i], k_mmul_left[i].in[0]);
      adf::connect<adf::window<window_branch_size>>(data_in1[i], k_mmul_left[i].in[1]);
      adf::connect<adf::window<window_data_size>>(k_mmul_left[i].out[0], data_out[i]);

      adf::location<adf::kernel>(k_mmul_left[i]) = adf::tile(start_col + i, start_row);
      adf::location<adf::stack>(k_mmul_left[i]) = adf::bank(start_col + i, start_row, 2);
      adf::location<adf::buffer>(k_mmul_left[i].in[0]) = adf::location<adf::kernel>(k_mmul_left[i]);
      adf::location<adf::buffer>(k_mmul_left[i].in[1]) = adf::location<adf::kernel>(k_mmul_left[i]);
      adf::location<adf::buffer>(k_mmul_left[i].out[0]) = {
        adf::address(start_col + i, start_row+1, 0x0),
        adf::address(start_col + i, start_row+1, 0x2000)
      };

      adf::runtime<ratio>(k_mmul_left[i]) = 1;
    }
  }

private:
  adf::kernel k_mmul_left[num_inputs];
};
