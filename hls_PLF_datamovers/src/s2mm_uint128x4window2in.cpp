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

    ap_uint<512> buffer = 0;
    hls::stream<ap_axiu<128,0,0,0>>* data_streams[] = {&s0, &s1, &s2, &s3};

    const unsigned int alignments_per_window = (window_size>>4); // (window_size/4 bytes per value)/4 values per window
    const unsigned int num_windows = (alignment_sites+alignments_per_window-1)/alignments_per_window;

    // calculate if alignment sites fits in the windows or if extention by zeroes is needed
    unsigned int quotient = alignment_sites/alignments_per_window;
    unsigned int product = quotient*alignments_per_window;
    unsigned int remainder = alignment_sites - product;
    if (remainder != 0) {
      //invert the remainder
      remainder = alignments_per_window-remainder;
    }

    // Receive alignment site data
    for(unsigned int i = 0; i < alignment_sites+remainder; i++) {
#pragma HLS PIPELINE II=1
      if (i < alignment_sites) {
        for(unsigned int j = 0; j < 4; j++) {
          ap_axiu<128,0,0,0> x = data_streams[j]->read();
          buffer.range(127+j*128, j*128) = x.data;
        }
        mem[i] = buffer;
      } else {
        for(unsigned int j = 0; j < 4; j++) {
          ap_axiu<128,0,0,0> x = data_streams[j]->read();
        }
      }
    }

  }

}


