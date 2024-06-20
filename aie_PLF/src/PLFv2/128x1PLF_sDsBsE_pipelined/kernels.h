
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

template <uint16_t data_window_size>
void mmul_branch(input_stream<float>* in_data, input_stream<float>* in_branch_matrix, output_stream<float>* out);

template <uint16_t data_window_size>
void combine_and_mac_EV(input_stream<float>* in_left, input_stream<float>* in_right, input_stream<float>* in_EV_matrix, output_stream<float>* out);

#endif
