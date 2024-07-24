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

  void convertFloatArrayToUint(float floatArray[16], ap_uint<512>& result) {
    // Allocate a temporary buffer to hold the serialized bits
    ap_uint<32> bitBuffer[16];

    // Serialize each float into the buffer
    for (size_t i = 0; i < 16; ++i) {
      // Convert the float to its IEEE 754 representation (32 bits)
      bitBuffer[i] = *(const ap_uint<32>*)(&floatArray[i]);
    }

    // Concatenate the bits in the buffer into a single 512-bit value
    for (size_t i = 0; i < sizeof(bitBuffer)/sizeof(bitBuffer[0]); ++i) {
      result.range(i*32, (i+1)*32 - 1) = bitBuffer[i];
    }
  }

  void mm2sleft(unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3, hls::stream<ap_axiu<128,0,0,0>> &sBranch0, hls::stream<ap_axiu<128,0,0,0>> &sBranch1, hls::stream<ap_axiu<128,0,0,0>> &sBranch2, hls::stream<ap_axiu<128,0,0,0>> &sBranch3, hls::stream<ap_axiu<128,0,0,0>> &sEV) {

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3
#pragma HLS interface axis port=sBranch0
#pragma HLS interface axis port=sBranch1
#pragma HLS interface axis port=sBranch2
#pragma HLS interface axis port=sBranch3
#pragma HLS interface axis port=sEV

#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // test data
    float data_arr[16] = {0.2135, 0.1427, 0.4139, 0.8301,
                          0.2021, 0.9124, 0.6542, 0.1235,
                          0.4856, 0.2242, 0.1322, 0.5223,
                          0.8223, 0.7741, 0.9855, 0.2024};

    // read ev matrix
    ap_uint<512> ev = 0;
    convertFloatArrayToUint(data_arr, ev);

    // read branch matrices and transpose them
    ap_uint<512> branchleft[4] = {0,0,0,0};
    convertFloatArrayToUint(data_arr, branchleft[0]);
    convertFloatArrayToUint(data_arr, branchleft[1]);
    convertFloatArrayToUint(data_arr, branchleft[2]);
    convertFloatArrayToUint(data_arr, branchleft[3]);

    ap_uint<512> buffer = 0;
    convertFloatArrayToUint(data_arr, buffer);

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

    // fill EV matrix
    // (16 elem * 32 = 512 bits / 512 bits = 1 mem read with 4 128-bit stream writes each (4 total))
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = ev.range(127 + j*128, j*128);
      sEV.write(x);
    }

    // Split the branch matrix and send them to each branch stream individually
    // (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = branchleft[0].range(127 + j*128, j*128);
      sBranch0.write(x);
    }
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = branchleft[1].range(127 + j*128, j*128);
      sBranch1.write(x);
    }
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = branchleft[2].range(127 + j*128, j*128);
      sBranch2.write(x);
    }
    for(unsigned int j = 0; j < 4; j++) {
      ap_axiu<128,0,0,0> x;
      x.data = branchleft[3].range(127 + j*128, j*128);
      sBranch3.write(x);
    }


    // Send alignment site data
    // per alignment site there are 16 values (4 streams each carrying 4 values per packet)
    for(unsigned int i = 0; i < alignment_sites; i++) {
#pragma HLS PIPELINE II=1

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
