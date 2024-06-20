/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

#define VECTOR_SIZE 4

template <uint16_t alignments>
void mmul_branch(input_stream<float>* in_data, input_window<float>* in_branch_matrix, output_stream<float>* out) {

  //const uint16_t alignments = 693;

  using MMUL = aie::mmul<4,4,1,float,float>;

  aie::vector<float, 16> branch;
  aie::vector<float, 4> data;

  branch = window_readincr_v<16>(in_branch_matrix);

  for (uint16_t i=0; i<alignments; i++) chess_prepare_for_pipelining {

    data = readincr_v<4>(in_data);

    MMUL matrix_mul;
    matrix_mul.mul(branch, data);

    writeincr(out, matrix_mul.to_vector<float>());

  }
}
