#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>
#include <math.h>

#define twotothe32  4294967296.0

#define minlikelihood  (1.0/twotothe32)



extern "C" {

  void unpack(ap_uint<512>& input, float outputArray[16]) {
#pragma HLS PIPELINE
    for (int i = 0; i < 16; i++) {
#pragma HLS UNROLL factor=16
      ap_uint<32> temp = input.range((i * 32) + 31, (i*32));
      outputArray[i] = *(const float*)(&temp);
    }
  } // void unpack()

  void pack(float floatArray[16], ap_uint<512>& result) {
#pragma HLS PIPELINE
    ap_uint<32> bitBuffer[16];

    for (size_t i = 0; i < 16; i++) {
#pragma HLS UNROLL factor=16
      bitBuffer[i] = *(const ap_uint<32>*)(&floatArray[i]);
      result.range((i*32)+31, i*32) = bitBuffer[i];
    }
  } // void pack()

  void scaling(ap_uint<512>* buffer, const unsigned int alignment_sites, const bool useFastScaling, int* wgt, int* addScale) {
#pragma HLS PIPELINE

    float x3[16];

    unpack(buffer, x3);

    unsigned int scale = 1;
    for(unsigned int l = 0; l < 16; l++) {
#pragma HLS UNROLL factor=16
      scale = (fabs(x3[l]) <  minlikelihood);
    }

    if (scale) {

      for (unsigned int l=0; l<16; l++) {
#pragma HLS UNROLL factor=16
        x3[l] *= twotothe32;
      }

      *addScale += *wgt;
    }

    pack(x3, buffer);

  } // void scaling()

} // extern "C"
