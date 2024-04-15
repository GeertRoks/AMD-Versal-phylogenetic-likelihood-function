/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

template <uint16_t window_size, uint16_t vector_size>
void window_passthrough_v(input_window<float>* in, output_window<float>* out) {
  const uint16_t blocks = (window_size>>2)/vector_size;
  for (uint16_t i=0; i<blocks; i++) {
    aie::vector<float, vector_size> a = window_readincr_v<vector_size>(in);
    window_writeincr(out, a);
  }
}
