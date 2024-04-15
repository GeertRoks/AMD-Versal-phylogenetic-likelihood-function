/*
   Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
   SPDX-License-Identifier: X11
   */


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  void s2mm(ap_uint<128>* mem, unsigned int size, hls::stream<ap_axiu<128,0,0,0>>& s0) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    const unsigned int iterations = size/4;
    for(unsigned int i = 0; i < iterations; i++) {
#pragma HLS PIPELINE II=1
      ap_axiu<128,0,0,0> x = s0.read();
      mem[i] = x.data;
    }
  }

}


