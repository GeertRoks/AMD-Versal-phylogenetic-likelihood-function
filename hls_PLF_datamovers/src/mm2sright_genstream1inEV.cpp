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

  void convertFloatArraytoUint(float floatArray[16], ap_uint<512>& result) {
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

  void mm2sright(unsigned int alignment_sites, unsigned int window_size, hls::stream<ap_axiu<128,0,0,0>> &s0, hls::stream<ap_axiu<128,0,0,0>> &s1, hls::stream<ap_axiu<128,0,0,0>> &s2, hls::stream<ap_axiu<128,0,0,0>> &s3) {

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3

#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // test data
    float data_arr[16] = {
      0.123456, 0.234567, 0.345678, 0.789543,
      0.456789, 0.567890, 0.678901, 0.789012,
      0.890123, 0.901234, 0.012345, 0.023456,
      0.034567, 0.045678, 0.056789, 0.067890
    };

    // read ev matrix
    ap_uint<512> ev = 0;
    convertFloatArraytoUint(data_arr, ev);

    // read branch matrices and transpose them
    ap_uint<512> branchleft[4] = {0,0,0,0};
    convertFloatArraytoUint(data_arr, branchleft[0]);
    convertFloatArraytoUint(data_arr, branchleft[1]);
    convertFloatArraytoUint(data_arr, branchleft[2]);
    convertFloatArraytoUint(data_arr, branchleft[3]);

    ap_uint<512> buffer = 0;
    convertFloatArraytoUint(data_arr, buffer);

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

    // load bottom half of EV matrix
    // (16 elem * 32 = 512 bits / 512 bits = 1 mem read with 4 128-bit stream writes each (4 total))
    ap_axiu<128,0,0,0> y;
    y.data = ev.range(383, 256);
    s0.write(y);
    s1.write(y);
    s2.write(y);
    s3.write(y);
    y.data = ev.range(511, 384);
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

      // give each data stream 4 data values of the 16 over a 128-bit stream ((128/8)/4 = 4 values)
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
