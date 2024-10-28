#include "plf.h"
#include <iostream>

#define ABS(x)    (((x)<0)   ?  (-(x)) : (x))
#define twotothe32  4294967296.0
#define minlikelihood  (1.0/twotothe32)

void plf(
    float *x1_start, float *x2_start, float *x3_start,
    float *EV, const int n, float *left, float *right,
    int* wgt, int& scalerIncrement
    )
{
  int i, j, k, l = 0;
  float *x1, *x2, *x3, ump_x1, ump_x2, x1px2[4];
  int addScale = 0;
  int scale = 0;

  for (i = 0; i < n; i++)
  {
    x1 = &x1_start[i * 16];
    x2 = &x2_start[i * 16];
    x3 = &x3_start[i * 16];

    for(j = 0; j < 16; j++) {
      x3[j] = 0.0;
    }

    for (j = 0; j < 4; j++) {
      for (k = 0; k < 4; k++) {

        ump_x1 = 0.0;
        ump_x2 = 0.0;

        for (l=0; l<4; l++) {
          //std::cout << "plf: x1[" << j*4+l << "]: " << x1[j*4+l] << std::endl;
          ump_x1 += x1[j*4 + l] * left[j*16 + k*4 +l];
          ump_x2 += x2[j*4 + l] * right[j*16 + k*4 +l];
        }

        x1px2[k] = ump_x1 * ump_x2;
        //std::cout << "plf: x1px2[" << k << "]: " << x1px2[k] << std::endl;
      }

      for(k = 0; k < 4; k++) {
        for (l=0; l<4; l++) {
          x3[j * 4 + l] +=  x1px2[k] * EV[4 * k + l];
          //std::cout << "plf: x3[" << j*4+l << "]: " << x3[j*4+l] << " = " << x1px2[k] << " * " << EV[4*k+l] << std::endl;
        }
      }

    }
    scale = 1;
    for(l = 0; scale && (l < 16); l++) {
      scale = (ABS(x3[l]) <  minlikelihood);
    }

    if(scale) {
      for (l=0; l<16; l++) {
        x3[l] *= twotothe32;
      }

      addScale += wgt[i];
    }
  }
  scalerIncrement = addScale;

}
