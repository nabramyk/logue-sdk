/*
    BSD 3-Clause License

    Copyright (c) 2018, KORG INC.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//*/

#include <thue_morse.hpp>

typedef struct State {
  float w0;
  float phase;
  float drive;
  float dist;
  float lfo, lfoz;
  uint8_t flags;
  uint8_t resolution;
  uint8_t next_val;
  int position_index;

  // For calculating the interpolated values between each point in the sequence, the interpolation will
  // begin at 'start' and finish at 'end'
  float start;
  float end;

} State;

static State s_state;

enum {
  k_flags_none = 0,
  k_flag_reset = 1<<0,
};

void OSC_INIT(uint32_t platform, uint32_t api)
{
  s_state.w0    = 0.f;
  s_state.phase = 0.f;
  s_state.drive = 1.f;
  s_state.dist  = 0.f;
  s_state.lfo = s_state.lfoz = 0.f;
  s_state.flags = k_flags_none;

  s_state.next_val = 0;
  s_state.position_index = 0;

  // Generate the first word
  for(; s_state.position_index<=7; s_state.position_index++) {
      s_state.next_val = thue_morse_seq_gen(s_state.position_index);

      s_state.resolution <<= 1;
      s_state.resolution |= s_state.next_val;
  }

  s_state.end = (float) ((int) s_state.resolution) / __UINT8_MAX__;
  s_state.end = sin(2*M_PI*s_state.end);
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{  
  const uint8_t flags = s_state.flags;
  s_state.flags = k_flags_none;
  
  const float w0 = s_state.w0 = osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF);
  float phase = (flags & k_flag_reset) ? 0.f : s_state.phase;
  
  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e = y + frames;
  
  // Index counter, starts at the 0 index of the sequence and counts up
  int position_index = 0;

  // The value of the sequence at position `counter`
  float current_it = 0.f;

  // The next value in the sequence, used for interpolating
  uint8_t next_val = 0;

  // The full byte sequence to store each successive bit value generated
  uint8_t resolution;

  float integral_counter = .00f;
  float integral_increment = .01f;

  float sig = 0.f;
  for (; y != y_e; ) {      
    if(integral_counter <= 1.f) {
      sig = cosintf(s_state.start, s_state.end, integral_counter);
      integral_counter += integral_increment;
    } else {
      // We've reached the end of the interpolation and need to recalculate the end point
      s_state.start = s_state.end;

      s_state.next_val = thue_morse_seq_gen(s_state.position_index);
      s_state.position_index++;

      s_state.resolution <<= 1;
      s_state.resolution |= s_state.next_val;
      s_state.end = (float) ((int) s_state.resolution) / __UINT8_MAX__;
      s_state.end = sin(2.f*3.1415f*s_state.end);
        
      integral_counter = integral_increment;
    }

    *(y++) = f32_to_q31(sig);
    
    phase += w0;
    phase -= (uint32_t)phase;
  }
  
  s_state.phase = phase;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  s_state.flags |= k_flag_reset;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
  const float valf = param_val_to_f32(value);
  
  switch (index) {
  case k_user_osc_param_id1:
  case k_user_osc_param_id2:
  case k_user_osc_param_id3:
  case k_user_osc_param_id4:
  case k_user_osc_param_id5:
  case k_user_osc_param_id6:
  case k_user_osc_param_shape:
  case k_user_osc_param_shiftshape:
  default:
    break;
  }
}

