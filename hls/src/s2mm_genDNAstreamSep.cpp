/*
   Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
   SPDX-License-Identifier: X11
   */


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  void s2mm(unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>>& s0, hls::stream<ap_axiu<128,0,0,0>>& s1, hls::stream<ap_axiu<128,0,0,0>>& s2, hls::stream<ap_axiu<128,0,0,0>>& s3) {
#pragma HLS PIPELINE

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3

#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // Add one padding alignment if alignments is odd, because read per 2 in AIE
    const unsigned int add_padding = (alignment_sites & 1);

    // Receive alignment site data
    for(unsigned int i = 0; i < alignment_sites+add_padding; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS loop_tripcount min=100 max=10000000 avg=100000

      ap_axiu<128,0,0,0> x[4];

      x[0] = s0.read();
      x[1] = s1.read();
      x[2] = s2.read();
      x[3] = s3.read();
    }

  } // void s2mm()

} // extern "C"
