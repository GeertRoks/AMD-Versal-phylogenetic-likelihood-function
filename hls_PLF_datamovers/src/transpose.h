#pragma once

#include <ap_int.h>

extern "C" {
  void transpose(const ap_uint<512> &input, ap_uint<512> &output);
}
