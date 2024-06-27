/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

#ifdef FALSE

template <uint16_t data_window_size>
void combine_and_mac_EV(input_window<float>* in_left, input_window<float>* in_right, input_window<float>* in_EV_matrix, output_window<float>* out) {

  //window_size (in bytes) div by 16 = num iterations to complete full window (div by 4 bytes per word and div by 4 elements per block = div by 16)
  const uint16_t blocks = (data_window_size>>4);
  //printf("mac called\n");

  aie::vector<float, 4> data_left;
  aie::vector<float, 4> data_right;

  aie::vector<float, 4> data_combined;

  aie::vector<float, 16> v_EV = window_readincr_v<16>(in_EV_matrix);
  aie::accum<accfloat, 4> acc1 = aie::zeros<accfloat, 4>();

  for (uint16_t i=0; i<blocks; i++) {

    // get data in
    data_left  = window_readincr_v<4>(in_left);
    data_right = window_readincr_v<4>(in_right);

    // combine left and right data by multiplying them together
    data_combined = aie::mul<accfloat,aie::vector<float, 4>,aie::vector<float, 4>>(data_left, data_right);

    // multiply and accumulate each row element of combined data with each element on the same row of the EV matrix.
    acc1 = aie::zeros<accfloat, 4>();
    for (uint16_t k=0; k<4; k++) {
      acc1 = aie::mac(acc1, v_EV.extract<4>(k), data_combined[k]);
    }

    // output result
    window_writeincr(out, acc1.to_vector<float>());

  }
}


template <uint16_t data_window_size>
void combine_and_mac_EV(input_window<float>* __restrict in_left, input_window<float>* __restrict in_right, input_window<float>* __restrict in_EV_matrix, output_window<float>* __restrict out) {

  //window_size (in bytes) div by 16 = num iterations to complete full window (div by 4 bytes per word and div by 4 elements per block = div by 16)
  const uint16_t blocks = (data_window_size>>5);

  using MMUL = aie::mmul<2,4,4,float,float>;
  MMUL matrix_mul;

  aie::vector<float, 8> data_left;
  aie::vector<float, 8> data_right;

  aie::vector<float, 8> data_combined;

  aie::vector<float, 16> v_EV = window_readincr_v<16>(in_EV_matrix);

  for (uint16_t i=0; i<blocks; i++)
  chess_prepare_for_pipelining
  chess_loop_range(blocks,0)
  {

    // get data in
    data_left  = window_readincr_v<8>(in_left);
    data_right = window_readincr_v<8>(in_right);

    // combine left and right data by multiplying them together
    data_combined = aie::mul(data_left, data_right);

    // apply EV matrix to combined data by matrix multiplication
    matrix_mul.mul(data_combined, v_EV);

    // output result
    window_writeincr(out, matrix_mul.to_vector<float>());

  }
}

#else

template <uint16_t data_window_size>
void combine_and_mac_EV(input_window<float>* __restrict in_left, input_window<float>* __restrict in_right,
                        input_window<float>* __restrict in_EV_matrix, output_window<float>* __restrict out) {

  const uint16_t blocks = (data_window_size>>5);
  using MMUL = aie::mmul<4,4,1,float,float>;
  MMUL matrix_mul;

  // EV is expected transposed
  auto v_EV = window_readincr_v<16>(in_EV_matrix);

  for (uint16_t i=0; i<blocks; i++) 
  chess_prepare_for_pipelining
  chess_loop_range(blocks,)
  {
    // get data in
    auto data_left  = window_readincr_v<8>(in_left);
    auto data_right = window_readincr_v<8>(in_right);

    // combine left and right data by multiplying them together
    aie::vector<float, 8> data = aie::mul(data_left, data_right);
    
    matrix_mul.mul(v_EV, data.extract<4>(0));
    window_writeincr(out, matrix_mul.to_vector<float>());
    matrix_mul.mul(v_EV, data.extract<4>(1));
    window_writeincr(out, matrix_mul.to_vector<float>());

  }
}

#endif
