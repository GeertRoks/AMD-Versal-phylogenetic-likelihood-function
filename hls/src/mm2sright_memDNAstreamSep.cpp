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

  void mm2sright(ap_uint<512>* mem, unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3, hls::stream<ap_axiu<128,0,0,0>> &sBranch0, hls::stream<ap_axiu<128,0,0,0>> &sBranch1, hls::stream<ap_axiu<128,0,0,0>> &sBranch2, hls::stream<ap_axiu<128,0,0,0>> &sBranch3) {
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

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // read branch matrices and transpose them
    ap_uint<512> branch[4] = {0,0,0,0};
    transpose(mem[0], branch[0]);
    transpose(mem[1], branch[1]);
    transpose(mem[2], branch[2]);
    transpose(mem[3], branch[3]);

    // Add one padding alignment if alignments is odd, because read per 2 in AIE
    unsigned int add_padding = (alignment_sites & 1);

    // Put the number of alignemnts in a 128-bit packet
    ap_axiu<128,0,0,0> alignments_packet;
    float alignments_float = static_cast<float>(alignment_sites + add_padding);
    ap_uint<128> itr;
    itr.range(31, 0) = *reinterpret_cast<ap_uint<32>*>(&alignments_float);
    alignments_packet.data = itr;

    // Send the alignments packet to each branch input
    sBranch0.write(alignments_packet);
    sBranch1.write(alignments_packet);
    sBranch2.write(alignments_packet);
    sBranch3.write(alignments_packet);


    // Split the branch matrix and send them to each branch stream individually
    // (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
    for(unsigned int j = 0; j < 4; j++) {
#pragma HLS PIPELINE II=1
      ap_axiu<128,0,0,0> x[4];
      x[0].data = branch[0].range(127 + j*128, j*128);
      x[1].data = branch[1].range(127 + j*128, j*128);
      x[2].data = branch[2].range(127 + j*128, j*128);
      x[3].data = branch[3].range(127 + j*128, j*128);
      sBranch0.write(x[0]);
      sBranch1.write(x[1]);
      sBranch2.write(x[2]);
      sBranch3.write(x[3]);
    }

    // Send alignment site data
    for(unsigned int i = 0; i < alignment_sites; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS loop_tripcount min=100 max=10000000 avg=100000

      // read all 16 values of one alignement
      // ((512 bits read/8 bits per byte)/4 bytes per float = 16 values)
      ap_uint<512> buffer = mem[4+i];

      // give each data stream 4 data values of the 16 over a 128-bit stream ((128/8)/4 = 4 values)
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

    if (add_padding) {
      ap_axiu<128,0,0,0> x;
      x.data = 0;
      s0.write(x);
      s1.write(x);
      s2.write(x);
      s3.write(x);
    }

  } // void mm2sright()

} //extern "C"
