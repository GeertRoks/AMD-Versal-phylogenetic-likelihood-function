#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

template <uint16_t data_window_size>
void combine(input_window<float>* __restrict in_left, input_window<float>* __restrict in_right,
              output_window<float>* __restrict out) {

  //window_size (in bytes) div by 32 = num iterations to complete full window
  //(div by 4 bytes per word and div by 8 elements per block = div by 32)
  const uint16_t blocks = (data_window_size>>5);

  aie::vector<float, 8> data_left;
  aie::vector<float, 8> data_right;
  aie::vector<float, 8> data_combined;

  // combine and passthrough the ev matrix
  aie::vector<float, 16> ev;
  ev.insert(0, window_readincr_v<8>(in_left));
  ev.insert(1, window_readincr_v<8>(in_right));
  window_writeincr(out, ev);

  for (uint16_t i=0; i<blocks; i++)
  chess_prepare_for_pipelining
  chess_loop_range(blocks,)
  {

    // get data in
    data_left  = window_readincr_v<8>(in_left);
    data_right = window_readincr_v<8>(in_right);

    // combine left and right data by multiplying them together
    data_combined = aie::mul(data_left, data_right);

    // output result
    window_writeincr(out, data_combined);

  }

}
