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

  void mm2sleft(ap_uint<512>* mem, unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // read ev matrix
    ap_uint<512> ev = mem[0];

    // read branch matrices and transpose them
    ap_uint<512> branchleft[4] = {0,0,0,0};
    transpose(mem[1], branchleft[0]);
    transpose(mem[2], branchleft[1]);
    transpose(mem[3], branchleft[2]);
    transpose(mem[4], branchleft[3]);

    // Add one padding alignment if alignments is odd, because read per 2 in AIE
    unsigned int add_padding = (alignment_sites & 1);

    // Put the number of alignemnts in a 128-bit packet
    ap_axiu<128,0,0,0> alignments_packet;
    float alignments_float = static_cast<float>(alignment_sites + add_padding);
    ap_uint<128> itr;
    itr.range(31, 0) = *reinterpret_cast<ap_uint<32>*>(&alignments_float);
    alignments_packet.data = itr;

    // Send the alignments packet to each branch input
    s0.write(alignments_packet);
    s1.write(alignments_packet);
    s2.write(alignments_packet);
    s3.write(alignments_packet);

    // load top half of EV matrix
    // (16 elem * 32 = 512 bits / 512 bits = 1 mem read with 4 128-bit stream writes each (4 total))
    ap_axiu<128,0,0,0> y;
    y.data = ev.range(127, 0);
    s0.write(y);
    s1.write(y);
    s2.write(y);
    s3.write(y);
    y.data = ev.range(255, 128);
    s0.write(y);
    s1.write(y);
    s2.write(y);
    s3.write(y);

    // Split the branch matrix and send them to each branch stream individually
    // (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = branchleft[0].range(127 + j*128, j*128);
      s0.write(x);
    }
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = branchleft[1].range(127 + j*128, j*128);
      s1.write(x);
    }
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = branchleft[2].range(127 + j*128, j*128);
      s2.write(x);
    }
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = branchleft[3].range(127 + j*128, j*128);
      s3.write(x);
    }


    // Send alignment site data
    // per alignment site there are 16 values (4 streams each carrying 4 values per packet)
    for(unsigned int i = 0; i < alignment_sites; i++) {
#pragma HLS PIPELINE II=1

      // read all 16 values of one alignement
      // ((512 bits read/8 bits per byte)/4 bytes per float = 16 values)
      ap_uint<512> buffer = mem[5+i];

      // give each data stream 4 of the 16 data values over a 128-bit stream ((128/8)/4 = 4 values)
      ap_axiu<128,0,0,0> x[4];
      x[0].data = buffer.range(127, 0);
      s0.write(x[0]);
      x[1].data = buffer.range(255, 128);
      s1.write(x[1]);
      x[2].data = buffer.range(383, 256);
      s2.write(x[2]);
      x[3].data = buffer.range(511, 384);
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

  }

}
