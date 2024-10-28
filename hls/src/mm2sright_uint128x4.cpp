/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  void mm2sright(ap_uint<512>* mem, unsigned int alignment_sites, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3, hls::stream<ap_axiu<128,0,0,0>> &sBranch) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3
#pragma HLS interface axis port=sBranch

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    ap_uint<512> buffer = 0;
    hls::stream<ap_axiu<128,0,0,0>>* s[] = {&s0, &s1, &s2, &s3};
    ap_axiu<128,0,0,0> x;

    // fill branch matrix (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
    for(unsigned int i = 0; i < 4; i++) {
      buffer = mem[i];
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch.write(x);
      }
    }

    for(unsigned int i = 0; i < alignment_sites; i++) {
#pragma HLS PIPELINE II=1
      buffer = mem[4+i];
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        s[j]->write(x);
      }
    }
  }

}
