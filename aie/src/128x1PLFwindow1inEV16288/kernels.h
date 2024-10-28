
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

template <uint16_t data_window_size>
void mmul_branch(input_window<float>* __restrict in_data, output_window<float>* __restrict out);

template <uint16_t data_window_size>
void combine(input_window<float>* __restrict in_left, input_window<float>* __restrict in_right, output_window<float>* __restrict out);

template <uint16_t data_window_size>
void ev(input_window<float>* __restrict in_data, output_window<float>* __restrict out);

#endif
