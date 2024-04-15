
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

template <uint16_t window_size, uint16_t vector_size>
void window_passthrough_v(input_window<float>* in, output_window<float>* out);

#endif
