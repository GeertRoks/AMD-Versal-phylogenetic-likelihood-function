#include "transpose.h"

extern "C" {

  // Function to transpose a 4x4 matrix represented as a single ap_uint<512> element
  void transpose(const ap_uint<512> &input, ap_uint<512> &output) {
    output.range( 31,   0) = input.range( 31,   0); //  0 ->  0
    output.range(159, 128) = input.range( 63,  32); //  1 ->  4
    output.range(287, 256) = input.range( 95,  64); //  2 ->  8
    output.range(415, 384) = input.range(127,  96); //  3 -> 12
    output.range( 63,  32) = input.range(159, 128); //  4 ->  1
    output.range(191, 160) = input.range(191, 160); //  5 ->  5
    output.range(319, 288) = input.range(223, 192); //  6 ->  9
    output.range(447, 416) = input.range(255, 224); //  7 -> 13
    output.range( 95,  64) = input.range(287, 256); //  8 ->  2
    output.range(223, 192) = input.range(319, 288); //  9 ->  6
    output.range(351, 320) = input.range(351, 320); // 10 -> 10
    output.range(479, 448) = input.range(383, 352); // 11 -> 14
    output.range(127,  96) = input.range(415, 384); // 12 ->  3
    output.range(255, 224) = input.range(447, 416); // 13 ->  7
    output.range(383, 352) = input.range(479, 448); // 14 -> 11
    output.range(511, 480) = input.range(511, 480); // 15 -> 15

  }

}
