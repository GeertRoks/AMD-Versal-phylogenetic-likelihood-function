/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

template <uint16_t alignments>
void combine_and_mac_EV(input_stream<float>* in_left, input_stream<float>* in_right, input_window<float>* in_EV_matrix, output_stream<float>* out) {

  //const uint16_t alignments = 693;

  aie::vector<float, 4> data_left;
  aie::vector<float, 4> data_right;

  aie::vector<float, 4> data_combined;

  aie::vector<float, 16> v_EV = window_readincr_v<16>(in_EV_matrix);
  aie::accum<accfloat, 4> acc1 = aie::zeros<accfloat, 4>();

  for (uint16_t i=0; i<alignments; i++) chess_prepare_for_pipelining {

    // get data in
    data_left  = readincr_v<4>(in_left);
    data_right = readincr_v<4>(in_right);

    // combine left and right data by multiplying them together
    data_combined = aie::mul<accfloat,aie::vector<float, 4>,aie::vector<float, 4>>(data_left, data_right);

    // multiply and accumulate each row element of combined data with each element on the same row of the EV matrix
    acc1 = aie::zeros<accfloat, 4>();
    for (uint16_t k=0; k<4; k++) {
      acc1 = aie::mac(acc1, v_EV.extract<4>(k), data_combined[k]);
    }

    // output result
    writeincr(out, acc1.to_vector<float>());

  }
}
