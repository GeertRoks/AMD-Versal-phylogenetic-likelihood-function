
#include "graph.h"

#define ITERATIONS 1

using namespace adf;

PLIOPassthroughGraph mygraph;

#if defined(__AIESIM__) || defined(__X86SIM__) || defined(__ADF_FRONTEND__)
int main(void) {
  mygraph.init();
  mygraph.run(ITERATIONS);
  mygraph.end();
  return 0;
}
#endif
