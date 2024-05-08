/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

template <uint16_t data_window_size>
void combine_and_mac_EV(input_window<float>* in_left, input_window<float>* in_right, input_window<float>* in_EV_matrix, input_stream<uint32_t>* in_alignments, output_window<float>* out) {

  // alignment sites div by 4 elements per block = num iterations to complete full sequence (round up to the next integer)
  const uint32_t alignment_sites = readincr(in_alignments);
  //const uint16_t blocks = ((alignment_sites+3)>>2)
  const uint16_t blocks = (data_window_size>>4);

  aie::vector<float, 4> data_left;
  aie::vector<float, 4> data_right;

  aie::vector<float, 4> data_combined;

  aie::vector<float, 16> v_EV = window_read_v<16>(in_EV_matrix);
  aie::accum<accfloat, 4> acc1 = aie::zeros<accfloat, 4>();

  for (uint16_t i=0; i<alignment_sites; i++) {

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
