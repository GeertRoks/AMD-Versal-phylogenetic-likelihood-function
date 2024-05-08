
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

template <uint16_t data_window_size>
void mmul_branch(input_window<float>* in_data, input_window<float>* in_branch_matrix, input_stream<uint32_t>* in_alignments, output_window<float>* out);

template <uint16_t data_window_size>
void combine_and_mac_EV(input_window<float>* in_left, input_window<float>* in_right, input_window<float>* in_EV_matrix, input_stream<uint32_t>* in_alignments, output_window<float>* out);

#endif
