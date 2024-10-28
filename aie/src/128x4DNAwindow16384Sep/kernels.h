
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

template <uint16_t data_window_size>
void mmul_branch(input_window<float>* __restrict in_data, input_window<float>* __restrict in_branch_matrix, output_window<float>* __restrict out);

template <uint16_t data_window_size>
void combine_and_mac_EV(input_window<float>* in_left, input_window<float>* in_right, input_window<float>* in_EV_matrix, output_window<float>* out);

template <uint16_t data_window_size>
void combine(input_window<float>* __restrict in_left, input_window<float>* __restrict in_right, output_window<float>* __restrict out);

template <uint16_t data_window_size>
void ev(input_window<float>* __restrict in_data, input_window<float>* __restrict in_EV_matrix, output_window<float>* __restrict out);

#endif