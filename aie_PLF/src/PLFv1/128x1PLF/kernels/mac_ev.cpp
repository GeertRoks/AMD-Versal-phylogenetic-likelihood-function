/* A simple kernel
 */
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

template <uint16_t data_window_size>
void mac_ev(input_window<float>* in_data, input_window<float>* in_EV_matrix, output_window<float>* out) {

  //div by 4 bytes per word and div by 16 elements per block
  const uint16_t blocks = ((data_window_size>>2)>>4);

  aie::vector<float, 16> v_EV;
  aie::vector<float, 4> v_data, v_out, v_acc;
  aie::accum<accfloat, 4> acc1 = aie::zeros<accfloat, 4>();
  aie::accum<accfloat, 4> acc2 = aie::zeros<accfloat, 4>();

  for (uint16_t i=0; i<blocks; i++) {

    v_EV = window_read_v<16>(in_EV_matrix);

    //for (uint16_t j=0; j<4; j++) {
    //  v_data = window_readincr_v<4>(in_data);
    //  for (uint16_t k=0; k<4; k++) {
    //    acc = aie::mul<accfloat>(v_data[k], v_EV.extract<4>(k*4));
    //    v_acc = acc.to_vector<float>();
    //    v_out[k] = aie::reduce_add(v_acc);
    //  }
    //  window_writeincr(out, v_out);
    //}

    // mac solution
    //v_EV = aie::transpose(v_EV, 4, 4);
    //for (uint16_t j=0; j<4; j++) {
    //  acc1 = aie::zeros<accfloat, 4>();
    //  acc2 = aie::zeros<accfloat, 4>();
    //  v_data = window_readincr_v<4>(in_data);
    //  for (uint16_t k=0; k<4; k+=2)
    //  chess_prepare_for_pipelining
    //  {
    //    acc1 = aie::mac(acc1, v_EV.extract<4>(k), v_data);
    //    acc2 = aie::mac(acc2, v_EV.extract<4>(k+1), v_data);
    //  }
    //  acc1 = aie::add(acc1, acc2.to_vector<float>());

    //  window_writeincr(out, acc1.to_vector<float>());
    //}
    v_EV = aie::transpose(v_EV, 4, 4);
    for (uint16_t j=0; j<4; j++) {
      acc1 = aie::zeros<accfloat, 4>();
      v_data = window_readincr_v<4>(in_data);
      for (uint16_t k=0; k<4; k++) {
        acc1 = aie::mac(acc1, v_EV.extract<4>(k), v_data);
      }
      window_writeincr(out, acc1.to_vector<float>());
    }

  }

}
