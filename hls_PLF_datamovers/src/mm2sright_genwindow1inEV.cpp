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
    ap_axiu<128,0,0,0> ev_packet[2];
    ev_packet[0].data = ev.range(383, 256);
    ev_packet[1].data = ev.range(511, 384);

    // read branch matrices and transpose them
    ap_uint<512> branch[4] = {0,0,0,0};
    convertFloatArraytoUint(data_arr, branch[0]);
    convertFloatArraytoUint(data_arr, branch[1]);
    convertFloatArraytoUint(data_arr, branch[2]);
    convertFloatArraytoUint(data_arr, branch[3]);

    // read alignment data
    ap_uint<512> buffer = 0;
    convertFloatArraytoUint(data_arr, buffer);

    // (window_size/4 bytes per value)/4 values per alignment -> div by 16
    const unsigned int alignments_per_window = (window_size>>4);
    const unsigned int num_full_windows = alignment_sites/alignments_per_window;
    const unsigned int remainder = alignment_sites - (num_full_windows*alignments_per_window);

    for(unsigned int window = 0; window < num_full_windows; window++) {
#pragma HLS PIPELINE II=1

      // load bottom half of EV matrix
      s0.write(ev_packet[0]);
      s1.write(ev_packet[0]);
      s2.write(ev_packet[0]);
      s3.write(ev_packet[0]);
      s0.write(ev_packet[1]);
      s1.write(ev_packet[1]);
      s2.write(ev_packet[1]);
      s3.write(ev_packet[1]);

      // Split the branch matrix and prepend them to each data stream
      // (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
      for(unsigned int j = 0; j < 4; j++) {
        ap_axiu<128,0,0,0> x;
        x.data = branch[0].range(127 + j*128, j*128);
        s0.write(x);
      }
      for(unsigned int j = 0; j < 4; j++) {
        ap_axiu<128,0,0,0> x;
        x.data = branch[1].range(127 + j*128, j*128);
        s1.write(x);
      }
      for(unsigned int j = 0; j < 4; j++) {
        ap_axiu<128,0,0,0> x;
        x.data = branch[2].range(127 + j*128, j*128);
        s2.write(x);
      }
      for(unsigned int j = 0; j < 4; j++) {
        ap_axiu<128,0,0,0> x;
        x.data = branch[3].range(127 + j*128, j*128);
        s3.write(x);
      }

      // Send branch matrix and alignment site data for a window
      for(unsigned int i = 0; i < alignments_per_window; i++) {
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
    }

    if (remainder > 0) {
#pragma HLS PIPELINE II=1
      // load bottom half of EV matrix
      s0.write(ev_packet[0]);
      s1.write(ev_packet[0]);
      s2.write(ev_packet[0]);
      s3.write(ev_packet[0]);
      s0.write(ev_packet[1]);
      s1.write(ev_packet[1]);
      s2.write(ev_packet[1]);
      s3.write(ev_packet[1]);

      // Split the branch matrix and prepend them to each data stream
      // (64 elem * 32 = 2048 bits / 512 bits = 4 mem reads with 4 128-bit stream writes each (16 total))
      for(unsigned int j = 0; j < 4; j++) {
        ap_axiu<128,0,0,0> x;
        x.data = branch[0].range(127 + j*128, j*128);
        s0.write(x);
      }
      for(unsigned int j = 0; j < 4; j++) {
        ap_axiu<128,0,0,0> x;
        x.data = branch[1].range(127 + j*128, j*128);
        s1.write(x);
      }
      for(unsigned int j = 0; j < 4; j++) {
        ap_axiu<128,0,0,0> x;
        x.data = branch[2].range(127 + j*128, j*128);
        s2.write(x);
      }
      for(unsigned int j = 0; j < 4; j++) {
        ap_axiu<128,0,0,0> x;
        x.data = branch[3].range(127 + j*128, j*128);
        s3.write(x);
      }
      for(unsigned int i = 0; i < remainder; i++) {
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
      // add zeroes to the input stream if alignments does not fit exactly in the last aie window
      for(unsigned int i = remainder; i < alignments_per_window; i++) {
        ap_axiu<128,0,0,0> x;
        x.data = 0;
        s0.write(x);
        s1.write(x);
        s2.write(x);
        s3.write(x);
      }
    }

  } // void mm2sright()

} // extern "C"
