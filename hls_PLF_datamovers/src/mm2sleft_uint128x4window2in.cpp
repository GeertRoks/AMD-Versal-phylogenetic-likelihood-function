/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: X11
*/


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>
#include "transpose.h"


extern "C" {

  void mm2sleft(ap_uint<512>* mem, unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3, hls::stream<ap_axiu<128,0,0,0>> &sBranch0, hls::stream<ap_axiu<128,0,0,0>> &sBranch1, hls::stream<ap_axiu<128,0,0,0>> &sBranch2, hls::stream<ap_axiu<128,0,0,0>> &sBranch3, hls::stream<ap_axiu<128,0,0,0>> &sEV) {
#pragma HLS PIPELINE
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

    // read ev matrix
    ap_uint<512> ev = mem[0];

    // read branch matrices and transpose them
    ap_uint<512> branch[4] = {0,0,0,0};
    transpose(mem[1], branch[0]);
    transpose(mem[2], branch[1]);
    transpose(mem[3], branch[2]);
    transpose(mem[4], branch[3]);

    // (window_size/4 bytes per value)/4 values per alignment -> div by 16
    const unsigned int alignments_per_window = (window_size>>4);
    const unsigned int num_full_windows = alignment_sites/alignments_per_window;
    const unsigned int remainder = alignment_sites - (num_full_windows*alignments_per_window);

    for(unsigned int window = 0; window < num_full_windows; window++) {
#pragma HLS PIPELINE
#pragma HLS loop_tripcount min=1 max=9765 avg=6000

      // fill EV matrix
      // (16 elem * 32 = 512 bits / 512 bits = 1 mem read with 4 128-bit stream writes each (4 total))
      // Split the branch matrix and send them to each branch stream individually
      // (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
      for(unsigned int j = 0; j < 4; j++) {
#pragma HLS PIPELINE II=1
        ap_axiu<128,0,0,0> x[5];
        x[0].data = ev.range(127 + j*128, j*128);
        x[1].data = branch[0].range(127 + j*128, j*128);
        x[2].data = branch[1].range(127 + j*128, j*128);
        x[3].data = branch[2].range(127 + j*128, j*128);
        x[4].data = branch[3].range(127 + j*128, j*128);
        sEV.write(x[0]);
        sBranch0.write(x[1]);
        sBranch1.write(x[2]);
        sBranch2.write(x[3]);
        sBranch3.write(x[4]);
      }

      // Send alignment site data for a window
      // per alignment site there are 16 values (4 streams each carrying 4 values per packet)
      for(unsigned int i = 0; i < alignments_per_window; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS loop_tripcount min=64 max=1018 avg=512
        // read all 16 values of one alignement
        // ((512 bits read/8 bits per byte)/4 bytes per float = 16 values)
        ap_uint<512> buffer = mem[5+(alignments_per_window*window)+i];
        // give each data stream 4 of the 16 data values over a 128-bit stream ((128/8)/4 = 4 values)
        ap_axiu<128,0,0,0> x[4];
        x[0].data = buffer.range(127, 0);
        x[1].data = buffer.range(255, 128);
        x[2].data = buffer.range(383, 256);
        x[3].data = buffer.range(511, 384);
        s0.write(x[0]);
        s1.write(x[1]);
        s2.write(x[2]);
        s3.write(x[3]);
      }
    }
    if (remainder > 0 ) {
      // fill EV matrix
      // (16 elem * 32 = 512 bits / 512 bits = 1 mem read with 4 128-bit stream writes each (4 total))
      // Split the branch matrix and send them to each branch stream individually
      // (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
      for(unsigned int j = 0; j < 4; j++) {
#pragma HLS PIPELINE II=1
        ap_axiu<128,0,0,0> x[5];
        x[0].data = ev.range(127 + j*128, j*128);
        x[1].data = branch[0].range(127 + j*128, j*128);
        x[2].data = branch[1].range(127 + j*128, j*128);
        x[3].data = branch[2].range(127 + j*128, j*128);
        x[4].data = branch[3].range(127 + j*128, j*128);
        sEV.write(x[0]);
        sBranch0.write(x[1]);
        sBranch1.write(x[2]);
        sBranch2.write(x[3]);
        sBranch3.write(x[4]);
      }

      // read the remaining alignments for the last window
      for(unsigned int i = 0; i < remainder; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS loop_tripcount min=1 max=1017 avg=509
        // read all 16 values of one alignement
        // ((512 bits read/8 bits per byte)/4 bytes per float = 16 values)
        ap_uint<512> buffer = mem[5+(alignments_per_window*num_full_windows)+i];
        // give each data stream 4 of the 16 data values over a 128-bit stream ((128/8)/4 = 4 values)
        ap_axiu<128,0,0,0> x[4];
        x[0].data = buffer.range(127, 0);
        x[1].data = buffer.range(255, 128);
        x[2].data = buffer.range(383, 256);
        x[3].data = buffer.range(511, 384);
        s0.write(x[0]);
        s1.write(x[1]);
        s2.write(x[2]);
        s3.write(x[3]);
      }
      // add zeroes to the input stream if alignments does not fit exactly in the last aie window
      for(unsigned int i = remainder; i < alignments_per_window; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS loop_tripcount min=1 max=1017 avg=508
        ap_axiu<128,0,0,0> x;
        x.data = 0;
        s0.write(x);
        s1.write(x);
        s2.write(x);
        s3.write(x);
      }
    } // if(remainder)

  } // void mm2sleft()

} // extern "C"

