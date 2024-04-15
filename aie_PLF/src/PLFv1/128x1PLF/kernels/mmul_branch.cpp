/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

#define VECTOR_SIZE 4

template <uint16_t data_window_size>
void mmul_branch(input_window<float>* in_data, input_window<float>* in_branch_matrix, output_window<float>* out) {
  const uint16_t blocks = ((data_window_size>>2)>>4); //div by 4 bytes per word and div by 16 elements per block
  using MMUL = aie::mmul<4,4,1,float,float>;

  for (uint16_t i=0; i<blocks; i++) {

    for (uint16_t j=0; j<4; j++) {
      aie::vector<float, 16> branch = window_readincr_v<16>(in_branch_matrix);
      aie::vector<float, 4> data = window_readincr_v<4>(in_data);
      MMUL matrix_mul;
      matrix_mul.mul(branch, data);
      aie::vector<float, 4> out_data = matrix_mul.to_vector<float>();

      window_writeincr(out, out_data);
    }

  }
}
