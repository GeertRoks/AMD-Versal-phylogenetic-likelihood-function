#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

void ev(input_stream<float>* __restrict in_data, input_stream<float>* __restrict in_EV_matrix,
        output_stream<float>* __restrict out) {

  using MMUL = aie::mmul<2,4,4,float,float>;
  MMUL matrix_mul;

  aie::vector<float, 8> data;
  aie::vector<float, 8> result;

  unsigned int alignments = readincr(in_data);

  // divide by two, because two alignments processed per loop iteration
  alignments = alignments >> 1;

  aie::vector<float, 16> v_EV = readincr_v<16>(in_EV_matrix);

  for (uint16_t i=0; i<alignments; i++)
  chess_prepare_for_pipelining
  {
    data = readincr_v<8>(in_data);

    matrix_mul.mul(data, v_EV);
    result = matrix_mul.to_vector<float>();

    writeincr(out, result.extract<4>(0));
    writeincr(out, result.extract<4>(1));
  }
}
