/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  void mm2sright(ap_uint<512>* mem, unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3, hls::stream<ap_axiu<128,0,0,0>> &sBranch0, hls::stream<ap_axiu<128,0,0,0>> &sBranch1, hls::stream<ap_axiu<128,0,0,0>> &sBranch2, hls::stream<ap_axiu<128,0,0,0>> &sBranch3) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3
#pragma HLS interface axis port=sBranch0
#pragma HLS interface axis port=sBranch1
#pragma HLS interface axis port=sBranch2
#pragma HLS interface axis port=sBranch3

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    ap_uint<512> buffer = 0;
    hls::stream<ap_axiu<128,0,0,0>>* data_streams[] = {&s0, &s1, &s2, &s3};
    //hls::stream<ap_axiu<128,0,0,0>>* branch_streams[] = {&sBranch0, &sBranch1, &sBranch2, &sBranch3};
    ap_axiu<128,0,0,0> x;

    const unsigned int alignments_per_window = (window_size>>4); // (window_size/4 bytes per value)/4 values per window
    const unsigned int num_windows = (alignment_sites+alignments_per_window-1)/alignments_per_window;

    // fill branch matrix (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
    for(unsigned int k = 0; k < num_windows; k++) {
      //for(unsigned int i = 0; i < 4; i++) {
      //  buffer = mem[i];
      //  for(unsigned int j = 0; j < 4; j++) {
      //    x.data = buffer.range(127 + j*128, j*128);
      //    branch_streams[i]->write(x);
      //  }
      //}
      buffer = mem[0];
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch0.write(x);
      }
      buffer = mem[1];
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch1.write(x);
      }
      buffer = mem[2];
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch2.write(x);
      }
      buffer = mem[3];
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch3.write(x);
      }
    }

    // calculate if alignment sites fits in the windows or if extention by zeroes is needed
    unsigned int quotient = alignment_sites/alignments_per_window;
    unsigned int product = quotient*alignments_per_window;
    unsigned int remainder = alignment_sites - product;
    if (remainder != 0) {
      //invert the remainder
      remainder = alignments_per_window-remainder;
    }

    // Send alignment site data
    for(unsigned int i = 0; i < alignment_sites+remainder; i++) {
#pragma HLS PIPELINE II=1
      if (i < alignment_sites) {
        // read all 16 values of one alignement ((512 bits read/8 bits per byte)/4 bytes per float = 16 values)
        buffer = mem[4+i];
        for(unsigned int j = 0; j < 4; j++) {
          // give each data stream 4 data values of the 16 over a 128-bit stream ((128/8)/4 = 4 values)
          x.data = buffer.range(127 + j*128, j*128);
          data_streams[j]->write(x);
        }
      } else {
        // add zeroes to the input stream if alignments does not fit exactly in the last aie window
        for(unsigned int j = 0; j < 4; j++) {
          x.data = 0;
          data_streams[j]->write(x);
        }
      }
    }

  }

}

