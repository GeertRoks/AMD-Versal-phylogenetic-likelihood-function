
#include <adf.h>
#include "kernels.h"

#define LANES_PER_GRAPH 4

template<uint16_t window_data_size, uint16_t window_branch_size, uint16_t window_half_ev_size>
class WindowPLFGraph : public adf::graph {
public:
  adf::port<input>  in_left_data[LANES_PER_GRAPH];
  adf::port<input>  in_right_data[LANES_PER_GRAPH];
  adf::port<output> out[LANES_PER_GRAPH];

  WindowPLFGraph(unsigned int start_col, unsigned int start_row) {

    for (unsigned int i = 0; i < LANES_PER_GRAPH; i++) {
      k_mmul_left[i] = adf::kernel::create(mmul_branch<window_data_size>);
      source(k_mmul_left[i]) = "kernels/mmul_branch.cpp";
      k_mmul_right[i] = adf::kernel::create(mmul_branch<window_data_size>);
      source(k_mmul_right[i]) = "kernels/mmul_branch.cpp";
      k_combine[i] = adf::kernel::create(combine<window_data_size>);
      source(k_combine[i]) = "kernels/combine.cpp";
      k_EV[i] = adf::kernel::create(ev<window_data_size>);
      source(k_EV[i]) = "kernels/ev.cpp";

      adf::connect< adf::window<window_data_size+window_branch_size+window_half_ev_size>   >(in_left_data[i],   k_mmul_left[i].in[0]);
      adf::connect< adf::window<window_data_size+window_branch_size+window_half_ev_size>   >(in_right_data[i],   k_mmul_right[i].in[0]);

      adf::connect< adf::window<window_data_size+window_half_ev_size>   >(k_mmul_left[i].out[0],  k_combine[i].in[0]);
      adf::connect< adf::window<window_data_size+window_half_ev_size>   >(k_mmul_right[i].out[0],  k_combine[i].in[1]);

      adf::connect< adf::window<window_data_size+window_half_ev_size+window_half_ev_size>   >(k_combine[i].out[0], k_EV[i].in[0]);

      adf::connect< adf::window<window_data_size>   >(k_EV[i].out[0], out[i]);

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
      adf::runtime<ratio>(k_combine[i]) = 1;
      adf::runtime<ratio>(k_EV[i]) = 1;
    }
  }

private:
  adf::kernel k_mmul_left[LANES_PER_GRAPH];
  adf::kernel k_mmul_right[LANES_PER_GRAPH];
  adf::kernel k_combine[LANES_PER_GRAPH];
  adf::kernel k_EV[LANES_PER_GRAPH];
};