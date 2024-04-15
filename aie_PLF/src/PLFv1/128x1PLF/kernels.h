
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

template <uint16_t data_window_size>
void mmul_branch(input_window<float>* in_data, input_window<float>* in_branch_matrix, output_window<float>* out);

template <uint16_t data_window_size>
void mul(input_window<float>* in_left, input_window<float>* in_right, output_window<float>* out);

#endif
