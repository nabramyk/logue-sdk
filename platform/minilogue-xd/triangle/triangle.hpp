#include "userosc.h"
#include <cstdlib>
#include <cmath>

  /**
   * Lookup value of triangle wave.
   *
   * @param   x  Phase ratio
   * @return     Result of (2 * abs(t - floor(t + 1/2))).
   */
  __fast_inline float osc_trif(float x) {
    const float p = x - (uint32_t) x;
    
    float y = floor(p + .5f);
    y = std::abs(p - y);
    y *= 2.f;

    return y;
  }