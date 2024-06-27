/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>


extern "C" {

  // Function to transpose a 4x4 matrix represented as a single ap_uint<512> element
  void transposeLeft(ap_uint<512> &input) {
    ap_uint<32> temp = 0;
    int idx1, idx2 = 0;
    for(int i = 0; i < 4; i++) {
      for (int j = i+1; j < 4; j++) {
        idx1 = (i*4+j)*32;
        idx2 = (j*4+i)*32;
        temp = input.range(idx2 + 31, idx2);
        input.range(idx2 + 31, idx2) = input.range(idx1 + 31, idx1);
        input.range(idx1 + 31, idx1) = temp;
      }
    }
  }

  void mm2sleft(ap_uint<512>* mem, unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3, hls::stream<ap_axiu<128,0,0,0>> &sBranch0, hls::stream<ap_axiu<128,0,0,0>> &sBranch1, hls::stream<ap_axiu<128,0,0,0>> &sBranch2, hls::stream<ap_axiu<128,0,0,0>> &sBranch3, hls::stream<ap_axiu<128,0,0,0>> &sEV) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3
#pragma HLS interface axis port=sBranch0
#pragma HLS interface axis port=sBranch1
#pragma HLS interface axis port=sBranch2
#pragma HLS interface axis port=sBranch3
#pragma HLS interface axis port=sEV

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // per alignment site there are 16 values (4 streams each carrying 4 values per packet)
    ap_uint<512> buffer = 0;
    ap_uint<512> buffer_intermediate = 0;
    hls::stream<ap_axiu<128,0,0,0>>* data_streams[] = {&s0, &s1, &s2, &s3};
    ap_axiu<128,0,0,0> x;

    const unsigned int alignments_per_window = (window_size>>4); // (window_size/4 bytes per value)/4 values per window
    const unsigned int num_windows = (alignment_sites+alignments_per_window-1)/alignments_per_window;

    // calculate if alignment sites fits in the windows or if extention by zeroes is needed
    unsigned int quotient = alignment_sites/alignments_per_window;
    unsigned int product = quotient*alignments_per_window;
    unsigned int remainder = alignment_sites - product;

    for(unsigned int window = 0; window < num_windows; window++) {

      // Split the branch matrix and send them to each branch stream individually
      // (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
      buffer = mem[0];
      transposeLeft(buffer);
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch0.write(x);
      }
      buffer = mem[1];
      transposeLeft(buffer);
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch1.write(x);
      }
      buffer = mem[2];
      transposeLeft(buffer);
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch2.write(x);
      }
      buffer = mem[3];
      transposeLeft(buffer);
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sBranch3.write(x);
      }


      // fill EV matrix
      // (16 elem * 32 = 512 bits / 512 bits = 1 mem read with 4 128-bit stream writes each (4 total))
      buffer = mem[4];
      for(unsigned int j = 0; j < 4; j++) {
        x.data = buffer.range(127 + j*128, j*128);
        sEV.write(x);
      }


      // Send alignment site data for a window
      for(unsigned int i = 0; i < alignments_per_window; i++) {
#pragma HLS PIPELINE II=1
        if (window >= num_windows-1 && remainder > 0 && i >= remainder) {
          // add zeroes to the input stream if alignments does not fit exactly in the last aie window
          for(unsigned int j = 0; j < 4; j++) {
            x.data = 0;
            data_streams[j]->write(x);
          }
        } else {
          // read all 16 values of one alignement
          // ((512 bits read/8 bits per byte)/4 bytes per float = 16 values)
          buffer = mem[5+(alignments_per_window*window)+i];
          for(unsigned int j = 0; j < 4; j++) {
            // give each data stream 4 data values of the 16 over a 128-bit stream ((128/8)/4 = 4 values)
            x.data = buffer.range(127 + j*128, j*128);
            data_streams[j]->write(x);
          }
        }
      }

    }

  }

}

