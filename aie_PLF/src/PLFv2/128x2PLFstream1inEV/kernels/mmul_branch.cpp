/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

void mmul_branch(input_stream<float>* __restrict in_data, output_stream<float>* __restrict out) {

  // in_data first has 4 values of which the first is the number of alignments,
  // then 8 values of half of the EV matrix and
  // then the 16 values of the branch matrix (transposed form)
  // afterwards comes all the alignment data

  using MMUL = aie::mmul<2,4,4,float,float>;
  MMUL matrix_mul;

  aie::vector<float, 16> branch;
  aie::vector<float, 8> data;
  aie::vector<float, 8> result;

  // read number of elements to be processed from branch input
  // passthrough the number of alignments
  aie::vector<float, 4> temp = readincr_v<4>(in_data);
  unsigned int alignments = static_cast<unsigned int>(temp[0]);
  writeincr(out, alignments);
  // divide by two, because two alignments processed per loop iteration
  alignments = alignments >> 1;

  // passthrough half of the ev matrix
  aie::vector<float, 8> ev;
  ev = readincr_v<8>(in_data);
  writeincr(out, ev);

  // branch is expected in transposed form
  branch = readincr_v<16>(in_data);

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
