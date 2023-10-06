#include <cstdlib>
#include <cstddef>
#include <unistd.h>
#include <cmath>

#include "userosc.h"

using namespace std;


float naive_lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

/***
 * Thue-Morse generator using the recursive principle
 * 
 * pos - the index value of the position in a binary sequence to calculate for
 */
int thue_morse_seq_gen(int pos) {
    if (pos == 0) {
        return pos;
    }

    if(pos % 2 == 0) { 
        return thue_morse_seq_gen(pos / 2);
    } else {
        return 1 - thue_morse_seq_gen((pos - 1) / 2);
    }
}

double degrees_to_radians(double degs) {
    return degs*PI/180;
}

  /**
   * Lookup value of triangle wave.
   *
   * @param   x  Phase ratio
   * @return     Result of (2 * abs(t - floor(t + 1/2))).
   */
  // __fast_inline float osc_thue_morsef(float x) {
  //   const float p = x - (uint32_t)x;
    
  //   float y = floor(p + .5f);
  //   y = std::abs(p - y);
  //   y *= 2.f;

  //   return y;
  // }