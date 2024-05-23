/*
   Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
   SPDX-License-Identifier: X11
   */


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  void s2mm(ap_uint<512>* mem, unsigned int alignment_sites, hls::stream<ap_axiu<128,0,0,0>>& s0, hls::stream<ap_axiu<128,0,0,0>>& s1, hls::stream<ap_axiu<128,0,0,0>>& s2, hls::stream<ap_axiu<128,0,0,0>>& s3) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    ap_uint<512> buffer = 0;
    hls::stream<ap_axiu<128,0,0,0>>* s[] = {&s0, &s1, &s2, &s3};

    // Receive alignment site data
    unsigned int quotient = alignment_sites>>6;
    unsigned int product = quotient<<6;
    unsigned int remainder = alignment_sites - product;
    remainder = 64-remainder;
    for(unsigned int i = 0; i < alignment_sites+remainder; i++) {
#pragma HLS PIPELINE II=1
      if (i < alignment_sites) {
        for(unsigned int j = 0; j < 4; j++) {
          ap_axiu<128,0,0,0> x = s[j]->read();
          buffer.range(127+j*128, j*128) = x.data;
        }
        mem[i] = buffer;
      } else {
        for(unsigned int j = 0; j < 4; j++) {
          ap_axiu<128,0,0,0> x = s[j]->read();
        }
      }
    }

    // empty the output stream if alignments does not fit exactly in the aie window
    //for(unsigned int i = 0; i < (alignment_sites%64); i++) {
    //  for(unsigned int j = 0; j < 4; j++) {
    //    ap_axiu<128,0,0,0> x = s[j]->read();
    //  }
    //}
  }

}


