/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  void convertFloatArraytoUint(float floatArray[16], ap_uint<512>& result) {
    // Allocate a temporary buffer to hold the serialized bits
    ap_uint<32> bitBuffer[16];

    // Serialize each float into the buffer
    for (size_t i = 0; i < 16; ++i) {
      // Convert the float to its IEEE 754 representation (32 bits)
      bitBuffer[i] = *(const ap_uint<32>*)(&floatArray[i]);
    }

    // Concatenate the bits in the buffer into a single 512-bit value
    for (size_t i = 0; i < sizeof(bitBuffer)/sizeof(bitBuffer[0]); ++i) {
      result.range(i*32, (i+1)*32 - 1) = bitBuffer[i];
    }
  }

  void mm2sright(unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3, hls::stream<ap_axiu<128,0,0,0>> &sBranch0, hls::stream<ap_axiu<128,0,0,0>> &sBranch1, hls::stream<ap_axiu<128,0,0,0>> &sBranch2, hls::stream<ap_axiu<128,0,0,0>> &sBranch3) {
#pragma HLS PIPELINE

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3
#pragma HLS interface axis port=sBranch0
#pragma HLS interface axis port=sBranch1
#pragma HLS interface axis port=sBranch2
#pragma HLS interface axis port=sBranch3

#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // test data
    float data_arr[16] = {
      0.123456, 0.234567, 0.345678, 0.789543,
      0.456789, 0.567890, 0.678901, 0.789012,
      0.890123, 0.901234, 0.012345, 0.023456,
      0.034567, 0.045678, 0.056789, 0.067890
    };

    // read alignment data
    ap_uint<512> buffer = 0;
    convertFloatArraytoUint(data_arr, buffer);

    ap_axiu<128,0,0,0> x[4];
    x[0].data = buffer.range(127, 0);
    x[1].data = buffer.range(255, 128);
    x[2].data = buffer.range(383, 256);
    x[3].data = buffer.range(511, 384);

    // (window_size/4 bytes per value)/4 values per alignment -> div by 16
    const unsigned int alignments_per_window = (window_size>>4);
    const unsigned int num_full_windows = alignment_sites/alignments_per_window;
    const unsigned int remainder = alignment_sites - (num_full_windows*alignments_per_window);
    const unsigned int extra_window = (remainder > 0);

    for(unsigned int window = 0; window < num_full_windows+extra_window; window++) {
#pragma HLS PIPELINE
#pragma HLS loop_tripcount min=0 max=9765 avg=6000

      for(unsigned int j = 0; j < 4; j++) {
#pragma HLS UNROLL FULL
        sBranch0.write(x[0]);
        sBranch1.write(x[1]);
        sBranch2.write(x[2]);
        sBranch3.write(x[3]);
      }

      for(unsigned int i = 0; i < alignments_per_window; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS loop_tripcount min=64 max=1018 avg=512
        s0.write(x[0]);
        s1.write(x[1]);
        s2.write(x[2]);
        s3.write(x[3]);
      }
    }

  } // void mm2sright()

} // extern "C"

