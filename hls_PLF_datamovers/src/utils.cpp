#include "utils.h"

extern "C" {

  // Function to transpose a 4x4 matrix represented as a single ap_uint<512> element
  void transpose(ap_uint<512> &input) {
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

}
