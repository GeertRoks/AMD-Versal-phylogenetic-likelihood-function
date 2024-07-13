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

    // (window_size/4 bytes per value)/4 values per alignment -> div by 16
    const unsigned int alignments_per_window = (window_size>>4);
    const unsigned int num_full_windows = alignment_sites/alignments_per_window;
    const unsigned int remainder = alignment_sites - (num_full_windows*alignments_per_window);

    // calculate if alignment sites fits in the windows or if extention by zeroes is needed
    //unsigned int quotient = alignment_sites/alignments_per_window;
    //unsigned int product = quotient*alignments_per_window;
    //unsigned int remainder = alignment_sites - product;
    //if (remainder != 0) {
    //  //invert the remainder
    //  remainder = alignments_per_window-remainder;
    //}

    // Receive alignment site data
    for(unsigned int window = 0; window < num_full_windows; window++) {
#pragma HLS PIPELINE II=1
      for(unsigned int i = 0; i < alignments_per_window; i++) {

        ap_axiu<128,0,0,0> x[4];
        ap_uint<512> buffer = 0;

        x[0] = s0.read();
        x[1] = s1.read();
        x[2] = s2.read();
        x[3] = s3.read();
        buffer.range(127, 0) = x[0].data;
        buffer.range(255, 128) = x[1].data;
        buffer.range(383, 256) = x[2].data;
        buffer.range(511, 384) = x[3].data;

        mem[(window*alignments_per_window)+i] = buffer;
      }
    }
    if (remainder > 0) {
      for(unsigned int i = 0; i < remainder; i++) {

        ap_axiu<128,0,0,0> x[4];
        ap_uint<512> buffer = 0;

        x[0] = s0.read();
        x[1] = s1.read();
        x[2] = s2.read();
        x[3] = s3.read();
        buffer.range(127, 0) = x[0].data;
        buffer.range(255, 128) = x[1].data;
        buffer.range(383, 256) = x[2].data;
        buffer.range(511, 384) = x[3].data;

        mem[(num_full_windows*alignments_per_window)+i] = buffer;
      }
      for(unsigned int i = remainder; i < alignments_per_window; i++) {
        ap_axiu<128,0,0,0> x[4];
        x[0] = s0.read();
        x[1] = s1.read();
        x[2] = s2.read();
        x[3] = s3.read();
      }
    }

  } // void s2mm()

} // extern "C"
