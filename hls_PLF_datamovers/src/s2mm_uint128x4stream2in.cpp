/*
   Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
   SPDX-License-Identifier: X11
   */


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  void s2mm(ap_uint<512>* mem, unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>>& s0, hls::stream<ap_axiu<128,0,0,0>>& s1, hls::stream<ap_axiu<128,0,0,0>>& s2, hls::stream<ap_axiu<128,0,0,0>>& s3) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // Add one padding alignment if alignments is odd, because read per 2 in AIE
    unsigned int add_padding = (alignment_sites & 1);

    // Receive alignment site data
    for(unsigned int i = 0; i < alignment_sites; i++) {
#pragma HLS PIPELINE II=1

      ap_axiu<128,0,0,0> x[4];
      ap_uint<512> buffer = 0;

      x[0] = s0.read();
      buffer.range(127, 0) = x[0].data;
      x[1] = s1.read();
      buffer.range(255, 128) = x[1].data;
      x[2] = s2.read();
      buffer.range(383, 256) = x[2].data;
      x[3] = s3.read();
      buffer.range(511, 384) = x[3].data;

      // write combined data to memory
      mem[i] = buffer;
    }

    if (add_padding) {
      ap_axiu<128,0,0,0> x[4];
      x[0] = s0.read();
      x[1] = s1.read();
      x[2] = s2.read();
      x[3] = s3.read();
    }

  }
}


