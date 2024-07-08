/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

void mmul_branch(input_stream<float>* __restrict in_data, input_stream<float>* __restrict in_branch_matrix, output_stream<float>* __restrict out) {

  using MMUL = aie::mmul<2,4,4,float,float>;
  MMUL matrix_mul;

  aie::vector<float, 16> branch;
  aie::vector<float, 8> data;
  aie::vector<float, 8> result;

  // read number of elements to be processed from branch input
  // passthrough the number of alignments
  aie::vector<float, 4> temp = readincr_v<4>(in_branch_matrix);
  unsigned int alignments = static_cast<unsigned int>(temp[0]);
  writeincr(out, alignments);
  // divide by two, because two alignments processed per loop iteration
  alignments = alignments >> 1;

  //branch is expected in transposed form
  branch = readincr_v<16>(in_branch_matrix);

  for (unsigned int i=0; i<alignments; i++)
  chess_prepare_for_pipelining
  {

    data = readincr_v<8>(in_data);

    matrix_mul.mul(data, branch);
    result = matrix_mul.to_vector<float>();

    writeincr(out, result.extract<4>(0));
    writeincr(out, result.extract<4>(1));

  }
}
