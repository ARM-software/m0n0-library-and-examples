
/*
 * Copyright (c) 2020, Arm Limited
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifndef KWS_MFCC_H
#define KWS_MFCC_H

#include "arm_math.h"

// used here
#include "custom_math.h"

#include "kws_constants.h"
// #include "custom_arm_rfft_q15.h"
#include "custom_arm_rfft_q31.h"

#define PREEMPH_COEFF Q15(0.97)
#define Q15(N) ((q15_t)(N*32768))
#define Q31(N) ((q31_t)(N*2147483648))

typedef struct signal_properties {
	int32_t offset;
	int16_t audio_min;
	int16_t audio_max;
	int16_t scale_shift;
} signal_properties_t;

//mfcc parameters computed at runtime
typedef struct mfcc_config {
	q31_t frame[FRAME_LEN_PADDED];
	q31_t fft [2*FRAME_LEN_PADDED];
	q31_t fft_abs2[FRAME_LEN_PADDED];
	q31_t mel_energies[NUM_FBANKS];
	// in ROM
	// q15_t window_func [FRAME_LEN];
	// int32_t fbank_filter_first[NUM_FBANKS];
	// int32_t fbank_filter_last[NUM_FBANKS];
	// q15_t ** mel_fbank;
	// q15_t dct_matrix[NUM_FBANKS*NUM_FBANKS];
	custom_arm_rfft_instance_q31 rfft;
} mfcc_config_t;

void mfcc_init(mfcc_config_t * m);
void mfcc_compute(mfcc_config_t * m, q15_t * data, q7_t * mfcc_out);

#endif
