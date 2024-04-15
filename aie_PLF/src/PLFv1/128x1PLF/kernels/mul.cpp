/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

template <uint16_t data_window_size>
void mul(input_window<float>* in_left, input_window<float>* in_right, output_window<float>* out) {
  const uint16_t blocks = ((data_window_size>>2)>>4); //div by 4 bytes per word and div by 16 elements per block
  for (uint16_t i=0; i<blocks; i++) {

    for (uint16_t j=0; j<4; j++) {
      aie::vector<float, 16> data_left  = window_readincr_v<16>(in_left);
      aie::vector<float, 16> data_right = window_readincr_v<16>(in_right);
      aie::vector<float, 16> out_data = aie::mul<accfloat,aie::vector<float, 16>,aie::vector<float, 16>>(data_left, data_right);
      window_writeincr(out, out_data);
    }

  }
}
