
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

void mmul_branch(input_stream<float>* __restrict in_data, input_stream<float>* __restrict in_branch_matrix, output_stream<float>* __restrict out);

void combine(input_stream<float>* __restrict in_left, input_stream<float>* __restrict in_right, output_stream<float>* __restrict out);

void ev(input_stream<float>* __restrict in_data, input_stream<float>* __restrict in_EV_matrix, output_stream<float>* __restrict out);

#endif
