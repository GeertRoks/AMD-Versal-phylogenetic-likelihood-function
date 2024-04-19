
#include <adf.h>
#include "kernels.h"

template<uint16_t window_data_size, uint16_t window_branch_size, uint16_t window_ev_size>
class WindowPLFGraph : public adf::graph {
public:
  adf::port<input>  data_in_left_data[4];
  adf::port<input>  data_in_left_branch[4];
  adf::port<input>  data_in_right_data[4];
  adf::port<input>  data_in_right_branch[4];
  adf::port<input>  data_in_EV[4];
  adf::port<output> data_out[4];

  WindowPLFGraph(unsigned int start_col, unsigned int start_row) {
    for (unsigned int i = 0; i < 4; i++) {
      k_mmul_left[i] = adf::kernel::create(mmul_branch<window_data_size>);
      source(k_mmul_left[i]) = "kernels/mmul_branch.cpp";
      k_mmul_right[i] = adf::kernel::create(mmul_branch<window_data_size>);
      source(k_mmul_right[i]) = "kernels/mmul_branch.cpp";
      k_mul_and_EV[i] = adf::kernel::create(combine_and_mac_EV<window_data_size>);
      source(k_mul_and_EV[i]) = "kernels/combine_and_mac_EV.cpp";

      adf::connect<adf::window<window_data_size>   >(data_in_left_data[i],   k_mmul_left[i].in[0]);
      adf::connect<adf::window<window_branch_size> >(data_in_left_branch[i], k_mmul_left[i].in[1]);
      adf::connect<adf::window<window_data_size>   >(k_mmul_left[i].out[0],  k_mul_and_EV[i].in[0]);

      adf::connect<adf::window<window_data_size>   >(data_in_right_data[i],   k_mmul_right[i].in[0]);
      adf::connect<adf::window<window_branch_size> >(data_in_right_branch[i], k_mmul_right[i].in[1]);
      adf::connect<adf::window<window_data_size>   >(k_mmul_right[i].out[0],  k_mul_and_EV[i].in[1]);

      adf::connect<adf::window<window_ev_size>>(data_in_EV[i], k_mul_and_EV[i].in[2]);

      adf::connect<adf::window<window_data_size>>(k_mul_and_EV[i].out[0], data_out[i]);

      //adf::location<adf::kernel>(k_mmul_left[i]) = adf::tile(start_col + i, start_row);
      //adf::location<adf::stack>(k_mmul_left[i]) = adf::bank(start_col + i, start_row, 2);
      //adf::location<adf::buffer>(k_mmul_left[i].in[0]) = adf::location<adf::kernel>(k_mmul_left[i]);
      //adf::location<adf::buffer>(k_mmul_left[i].in[1]) = adf::location<adf::kernel>(k_mmul_left[i]);
      //adf::location<adf::buffer>(k_mmul_left[i].out[0]) = {
      //  adf::address(start_col + i, start_row+1, 0x0),
      //  adf::address(start_col + i, start_row+1, 0x2000)
      //};

      adf::runtime<ratio>(k_mmul_left[i]) = 1;
      adf::runtime<ratio>(k_mmul_right[i]) = 1;
      adf::runtime<ratio>(k_mul_and_EV[i]) = 1;
    }
  }

private:
  adf::kernel k_mmul_left[4];
  adf::kernel k_mmul_right[4];
  adf::kernel k_mul_and_EV[4];
};
