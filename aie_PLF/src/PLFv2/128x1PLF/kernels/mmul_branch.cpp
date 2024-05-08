/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

#define VECTOR_SIZE 4

template <uint16_t data_window_size>
void mmul_branch(input_window<float>* in_data, input_window<float>* in_branch_matrix, input_stream<uint32_t>* in_alignments, output_window<float>* out) {

  // alignment sites div by 4 elements per block = num iterations to complete full sequence (round up to the next integer)
  const uint32_t alignment_sites = readincr(in_alignments);
  //const uint16_t blocks = ((alignment_sites+3)>>2)
  const uint16_t blocks = (data_window_size>>4);

  using MMUL = aie::mmul<4,4,1,float,float>;

  aie::vector<float, 16> branch;
  aie::vector<float, 4> data;

  branch = window_readincr_v<16>(in_branch_matrix);
  for (uint16_t i=0; i<alignment_sites; i++) {

    data = window_readincr_v<4>(in_data);

    MMUL matrix_mul;
    matrix_mul.mul(branch, data);

    window_writeincr(out, matrix_mul.to_vector<float>());

  }
}
