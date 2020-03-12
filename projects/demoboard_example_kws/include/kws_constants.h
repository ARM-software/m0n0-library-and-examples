
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
#ifndef KWS_CONSTANTS
#define KWS_CONSTANTS

// Constants shared between kwc, mfcc and nn files
#define M_2PI 6.28318530717958647693f

// System constants
#define AUDIO_FREQ 8000
#define AUDIO_LENGTH_MS 1000
#define FRAME_LENGTH_MS 62.5
#define FRAME_SHIFT_MS 62.5
#define FRAME_LEN 500
#define FRAME_LEN_PADDED 512
#define FRAME_SHIFT 500
#define NUM_AUDIO_WIN_INPUT 2
#define NUM_AUDIO_WIN_TOTAL 16

// MFCC Constants
#define NUM_FBANKS 40
#define NUM_MFCC_COEFFS 8
#define MFCC_BUFFER_SIZE 128

// NN Constants
#define IN_DIM 128
#define OUT_DIM 12
#define L_0_OUT_DIM 128
#define L_1_OUT_DIM 128
#define L_0_W_DIM 16384
#define L_1_W_DIM 16384
#define L_2_W_DIM 1536

#endif
