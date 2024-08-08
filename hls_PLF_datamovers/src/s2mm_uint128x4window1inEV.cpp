/*
   Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
   SPDX-License-Identifier: X11
   */


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>
#include <math.h>

#define twotothe32  4294967296.0

#define minlikelihood  (1.0/twotothe32)

#define WGT_SIZE 16384

extern "C" {

  void s2mm(ap_uint<512>* mem, unsigned int alignment_sites, unsigned int window_size, unsigned int fill_wgt, hls::stream<ap_axiu<128,0,0,0>>& s0, hls::stream<ap_axiu<128,0,0,0>>& s1, hls::stream<ap_axiu<128,0,0,0>>& s2, hls::stream<ap_axiu<128,0,0,0>>& s3) {
#pragma HLS PIPELINE

#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=alignment_sites bundle=control
#pragma HLS INTERFACE s_axilite port=window_size bundle=control
#pragma HLS INTERFACE s_axilite port=fill_wgt bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // (window_size/4 bytes per value)/4 values per alignment -> div by 16
    const unsigned int alignments_per_window = (window_size>>4);
    const unsigned int num_full_windows = alignment_sites/alignments_per_window;
    // calculate if alignment sites fits in the windows or if extention by zeroes is needed
    const unsigned int remainder = alignment_sites - (num_full_windows*alignments_per_window);
    const unsigned int extra_window = (remainder > 0);

    static int wgt[WGT_SIZE];
#pragma HLS BIND_STORAGE variable=wgt type=ram_2p impl=bram

    int addScale = 0;
    bool useFastScaling = 1;
    int scalerIncrement = 0;

    if (fill_wgt) {
      for (unsigned int j = 0; j < (WGT_SIZE>>4); j++) {
#pragma HLS PIPELINE II=8
        ap_uint<512> temp = mem[j];
        for (unsigned int i = 0; i < 16; i++) {
#pragma HLS UNROLL factor=2 skip_exit_check
          wgt[(j*16)+i] = temp.range(15+(i*16), i*16);
        }
      }
    } else {
      // Receive alignment site data
      for(unsigned int window = 0; window < num_full_windows + extra_window; window++) {
#pragma HLS PIPELINE
#pragma HLS loop_tripcount min=1 max=9765 avg=6000
        for(unsigned int i = 0; i < alignments_per_window; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS loop_tripcount min=64 max=1018 avg=512

          // read stream data
          ap_axiu<128,0,0,0> x[4];
          x[0] = s0.read();
          x[1] = s1.read();
          x[2] = s2.read();
          x[3] = s3.read();

          // unpack stream data -> float array
          float x3[16];
          for (int i = 0; i < 4; i++) {
#pragma HLS UNROLL factor=4
            for (int j = 0; j < 4; j++) {
#pragma HLS UNROLL factor=4
              ap_uint<32> temp = x[j].data.range((i * 32) + 31, (i*32));
              x3[j*4 + i] = *(const float*)(&temp);
            }
          }

          // check if scaling is needed
          ap_uint<16> scale = 0;
          for(unsigned int l = 0; l < 16; l++) {
#pragma HLS UNROLL factor=16
            scale.range(l,l) = !(fabs(x3[l]) <  minlikelihood);
          }

          // perform scaling when needed
          if (scale==0) {
            for (unsigned int l=0; l<16; l++) {
#pragma HLS UNROLL factor=16
              x3[l] *= twotothe32;
            }
            addScale += wgt[(window*alignments_per_window)+i];
          }

          //pack x3 -> buffer
          ap_uint<512> buffer = 0;
          for (size_t i = 0; i < 16; i++) {
#pragma HLS UNROLL factor=16
            ap_uint<32> bitBuffer = *(const ap_uint<32>*)(&x3[i]);
            buffer.range((i*32)+31, i*32) = bitBuffer;
          }

          // write buffer to memory
          mem[(window*alignments_per_window)+i] = buffer;
        }
      }
    }

  } // void s2mm()

} // extern "C"
