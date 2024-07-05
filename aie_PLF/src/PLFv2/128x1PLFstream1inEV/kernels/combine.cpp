#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

void combine(input_stream<float>* __restrict in_left, input_stream<float>* __restrict in_right,
              output_stream<float>* __restrict out) {

  aie::vector<float, 8> data_left;
  aie::vector<float, 8> data_right;
  aie::vector<float, 8> data_combined;

  // read from both inputs the alignments (they should be the same)
  // passthrough number of alignments
  unsigned int alignments = readincr(in_left);
  alignments = readincr(in_right);
  writeincr(out, alignments);
  // divide by two, because two alignments processed per loop iteration
  alignments = alignments >> 1;

  // combine and passthrough the ev matrix
  aie::vector<float, 16> ev;
  ev.insert(0, readincr_v<8>(in_left));
  ev.insert(1, readincr_v<8>(in_right));
  writeincr(out, ev);

  for (unsigned int i=0; i<alignments; i++)
  chess_prepare_for_pipelining
  {

    // get data in
    data_left  = readincr_v<8>(in_left);
    data_right = readincr_v<8>(in_right);

    // combine left and right data by multiplying them together
    data_combined = aie::mul(data_left, data_right);

    // output result
    writeincr(out, data_combined.extract<4>(0));
    writeincr(out, data_combined.extract<4>(1));

  }

}
