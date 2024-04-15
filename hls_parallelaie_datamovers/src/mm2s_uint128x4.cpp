/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  void mm2s(ap_uint<512>* mem, unsigned int size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    const unsigned int iterations = (size>>4);
    hls::stream<ap_axiu<128,0,0,0>>* s[] = {&s0, &s1, &s2, &s3};
    for(unsigned int i = 0; i < iterations; i++) {
#pragma HLS PIPELINE II=1
      ap_uint<128>* mem_128 = reinterpret_cast<ap_uint<128>*>(&mem[i]);
      for(unsigned int j = 0; j < 4; i++) {
        ap_axiu<128,0,0,0> x;
        x.data = mem_128 + j;
        s[j]->write(x);
      }
    }
  }

}

