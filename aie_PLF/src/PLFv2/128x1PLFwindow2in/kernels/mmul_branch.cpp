/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

template <uint16_t data_window_size>
void mmul_branch(input_window<float>* __restrict in_data, input_window<float>* __restrict in_branch_matrix, output_window<float>* __restrict out) {

  //window_size (in bytes) div by 32 = num iterations to complete full window
  //(div by 4 bytes per word and div by 2*4 elements per block = div by 32)
  const uint16_t blocks = (data_window_size>>5);

  using MMUL = aie::mmul<2,4,4,float,float>;
  MMUL matrix_mul;

  aie::vector<float, 16> branch;
  aie::vector<float, 8> data;

  //branch is expected in transposed form
  branch = window_readincr_v<16>(in_branch_matrix);

  for (uint16_t i=0; i<blocks; i++)
  chess_prepare_for_pipelining
  chess_loop_range(blocks,)
  {

    data = window_readincr_v<8>(in_data);

    matrix_mul.mul(data, branch);

    window_writeincr(out, matrix_mul.to_vector<float>());

  }
}
