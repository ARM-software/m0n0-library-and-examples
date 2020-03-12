
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
/**
 * File: mfcc.c
 * Description: MFCC computation, derived from parts of Kaldi and David Palframan
 *
 * Author: Fernando García Redondo
 * Date: 10/25/18
 */

#include "mfcc.h"
// System constants
// All rom stuff
#include "mfcc_rom_constants.h"

/*
	Constants
*/
// log_32(2**11);, being 11 mel_n bits with fft^2 as the selected magnitude
const int32_t LOG_ENERGY_CONSTANT = -227130; // log_32(2**11)
// const int32_t LOG_ENERGY_CONSTANT = -317982; // log_32(2**9)

// const int16_t FFT_SCALE_UP = 8; //WARNING: hardcoded for frame_len_padded == 512
const uint32_t HALF_DIM = (uint32_t)FRAME_LEN_PADDED/2 + 1;

const q15_t WINDOW_FUNC[] = DEF_WINDOW_FUNC_FL_500;
const q15_t DCT_MATRIX[] = DEF_DCT_LIFTER_MATRIX_NB_40_NC_8;
const uint32_t FBANK_FILTER_FIRST[] = DEF_MEL_FBANK_FIRST_NB_40;
const uint32_t FBANK_FILTER_LAST[] = DEF_MEL_FBANK_LAST_NB_40;

const q31_t MEL_FBANK_NB_40_0[] = DEF_MEL_FBANK_NB_40_0;
const q31_t MEL_FBANK_NB_40_1[] = DEF_MEL_FBANK_NB_40_1;
const q31_t MEL_FBANK_NB_40_2[] = DEF_MEL_FBANK_NB_40_2;
const q31_t MEL_FBANK_NB_40_3[] = DEF_MEL_FBANK_NB_40_3;
const q31_t MEL_FBANK_NB_40_4[] = DEF_MEL_FBANK_NB_40_4;
const q31_t MEL_FBANK_NB_40_5[] = DEF_MEL_FBANK_NB_40_5;
const q31_t MEL_FBANK_NB_40_6[] = DEF_MEL_FBANK_NB_40_6;
const q31_t MEL_FBANK_NB_40_7[] = DEF_MEL_FBANK_NB_40_7;
const q31_t MEL_FBANK_NB_40_8[] = DEF_MEL_FBANK_NB_40_8;
const q31_t MEL_FBANK_NB_40_9[] = DEF_MEL_FBANK_NB_40_9;
const q31_t MEL_FBANK_NB_40_10[] = DEF_MEL_FBANK_NB_40_10;
const q31_t MEL_FBANK_NB_40_11[] = DEF_MEL_FBANK_NB_40_11;
const q31_t MEL_FBANK_NB_40_12[] = DEF_MEL_FBANK_NB_40_12;
const q31_t MEL_FBANK_NB_40_13[] = DEF_MEL_FBANK_NB_40_13;
const q31_t MEL_FBANK_NB_40_14[] = DEF_MEL_FBANK_NB_40_14;
const q31_t MEL_FBANK_NB_40_15[] = DEF_MEL_FBANK_NB_40_15;
const q31_t MEL_FBANK_NB_40_16[] = DEF_MEL_FBANK_NB_40_16;
const q31_t MEL_FBANK_NB_40_17[] = DEF_MEL_FBANK_NB_40_17;
const q31_t MEL_FBANK_NB_40_18[] = DEF_MEL_FBANK_NB_40_18;
const q31_t MEL_FBANK_NB_40_19[] = DEF_MEL_FBANK_NB_40_19;
const q31_t MEL_FBANK_NB_40_20[] = DEF_MEL_FBANK_NB_40_20;
const q31_t MEL_FBANK_NB_40_21[] = DEF_MEL_FBANK_NB_40_21;
const q31_t MEL_FBANK_NB_40_22[] = DEF_MEL_FBANK_NB_40_22;
const q31_t MEL_FBANK_NB_40_23[] = DEF_MEL_FBANK_NB_40_23;
const q31_t MEL_FBANK_NB_40_24[] = DEF_MEL_FBANK_NB_40_24;
const q31_t MEL_FBANK_NB_40_25[] = DEF_MEL_FBANK_NB_40_25;
const q31_t MEL_FBANK_NB_40_26[] = DEF_MEL_FBANK_NB_40_26;
const q31_t MEL_FBANK_NB_40_27[] = DEF_MEL_FBANK_NB_40_27;
const q31_t MEL_FBANK_NB_40_28[] = DEF_MEL_FBANK_NB_40_28;
const q31_t MEL_FBANK_NB_40_29[] = DEF_MEL_FBANK_NB_40_29;
const q31_t MEL_FBANK_NB_40_30[] = DEF_MEL_FBANK_NB_40_30;
const q31_t MEL_FBANK_NB_40_31[] = DEF_MEL_FBANK_NB_40_31;
const q31_t MEL_FBANK_NB_40_32[] = DEF_MEL_FBANK_NB_40_32;
const q31_t MEL_FBANK_NB_40_33[] = DEF_MEL_FBANK_NB_40_33;
const q31_t MEL_FBANK_NB_40_34[] = DEF_MEL_FBANK_NB_40_34;
const q31_t MEL_FBANK_NB_40_35[] = DEF_MEL_FBANK_NB_40_35;
const q31_t MEL_FBANK_NB_40_36[] = DEF_MEL_FBANK_NB_40_36;
const q31_t MEL_FBANK_NB_40_37[] = DEF_MEL_FBANK_NB_40_37;
const q31_t MEL_FBANK_NB_40_38[] = DEF_MEL_FBANK_NB_40_38;
const q31_t MEL_FBANK_NB_40_39[] = DEF_MEL_FBANK_NB_40_39;

const q31_t* MEL_FBANK[] = {&MEL_FBANK_NB_40_0[0], &MEL_FBANK_NB_40_1[0], &MEL_FBANK_NB_40_2[0], &MEL_FBANK_NB_40_3[0], &MEL_FBANK_NB_40_4[0], &MEL_FBANK_NB_40_5[0], &MEL_FBANK_NB_40_6[0], &MEL_FBANK_NB_40_7[0], &MEL_FBANK_NB_40_8[0], &MEL_FBANK_NB_40_9[0], &MEL_FBANK_NB_40_10[0], &MEL_FBANK_NB_40_11[0], &MEL_FBANK_NB_40_12[0], &MEL_FBANK_NB_40_13[0], &MEL_FBANK_NB_40_14[0], &MEL_FBANK_NB_40_15[0], &MEL_FBANK_NB_40_16[0], &MEL_FBANK_NB_40_17[0], &MEL_FBANK_NB_40_18[0], &MEL_FBANK_NB_40_19[0], &MEL_FBANK_NB_40_20[0], &MEL_FBANK_NB_40_21[0], &MEL_FBANK_NB_40_22[0], &MEL_FBANK_NB_40_23[0], &MEL_FBANK_NB_40_24[0], &MEL_FBANK_NB_40_25[0], &MEL_FBANK_NB_40_26[0], &MEL_FBANK_NB_40_27[0], &MEL_FBANK_NB_40_28[0], &MEL_FBANK_NB_40_29[0], &MEL_FBANK_NB_40_30[0], &MEL_FBANK_NB_40_31[0], &MEL_FBANK_NB_40_32[0], &MEL_FBANK_NB_40_33[0], &MEL_FBANK_NB_40_34[0], &MEL_FBANK_NB_40_35[0], &MEL_FBANK_NB_40_36[0], &MEL_FBANK_NB_40_37[0], &MEL_FBANK_NB_40_38[0], &MEL_FBANK_NB_40_39[0]};

/*
	Private functions available to the user
*/

/*
	Hamming window
* 	preemphasis, dc-removal or scaling is not done
*/
void signal_windowing(q15_t* data)
{
	// optimized
	arm_mult_q15(data, (q15_t*) WINDOW_FUNC, data, FRAME_LEN);
}

/**
	@brief Computes mel energies
	@param mfcc_config_t* m is the pointer to the mfcc structure.
	
	Inputs are
		q31_t (Q31) MEL_FBANK, 
		q31_t (Q20.12) FFT2
		or q31_t (Q10.22) FFT_abs

	Output in m->mel_engergies buffer is Q21.11 if fft2 was input
	or Q14.18 if fft_abs was input
*/
void compute_mel_energies(mfcc_config_t * m)
{
	// Apply mel filterbanks
	for (uint32_t bin = 0; bin < (uint32_t)NUM_FBANKS; bin++) {
		q63_t mel_energy = 0;
		uint32_t first_index = FBANK_FILTER_FIRST[bin];
		uint32_t last_index = FBANK_FILTER_LAST[bin];
		uint32_t length = last_index-first_index+1;

		// total downscaling: 4 + 31
		// Internal downscaling could be mel_pre_downscaling = 4 instead of 14b
		// Idea: custom arm_dot_prod_q31 with pre_downscaling as parameter
		// The return result is in 16.48
		// https://www.keil.com/pack/doc/CMSIS/DSP/html/group__dot__prod.html#gab15d8fa060fc85b4d948d091b7deaa11
		custom_arm_dot_prod_q31(&m->fft_abs2[first_index],
			(q31_t*) MEL_FBANK[bin], length, &mel_energy, 4);
		// meet internal downscaling, instead
		// >> 3 + 31, we do
		m->mel_energies[bin] = (q31_t) (mel_energy >> 31);

		//avoid log of zero
		if (m->mel_energies[bin] == 0){
			m->mel_energies[bin] = 1;
		}
	}
}

/**
	@brief Computes log of the  mel energies
	@param mfcc_config_t* m is the pointer to the mfcc structure.
	
	Inputs are
		q31_t (Q21.11 or Q14.18) (mel_energies buffer) depending on whether fft2
		or fft_abs was performeds

	Output in m->mel_engergies buffer is Q16.16
*/
void compute_log_mel_energies(mfcc_config_t * m)
{
	// output is Q16.16, no downscale needed.
	// change LOG_ENERGY_CONSTANT accordingly to mel_n bits
	for (uint32_t bin = 0; bin < (uint32_t)NUM_FBANKS; bin++) {
		m->mel_energies[bin] = (log_32(m->mel_energies[bin]) - LOG_ENERGY_CONSTANT);
	}
}


/**
	@brief Computes mel energies and their log altogether
	@param mfcc_config_t* m is the pointer to the mfcc structure.
	
	Inputs are
		q31_t (Q31) MEL_FBANK, 
		q31_t (Q20.12) FFT2
		or q31_t (Q10.22) FFT_abs

	Internal buffer in m->mel_engergies buffer is Q21.11 if fft2 was input
	or Q14.18 if fft_abs was input
	Output in m->mel_engergies buffer is Q16.16
*/
void compute_mel_and_log_energies_together(mfcc_config_t * m)
{

	//////////////////////////
	// Mel & log all together
	//////////////////////////
	q63_t mel_energy = 0;
	uint32_t first_index = 0;
	uint32_t last_index = 0;
	uint32_t length = 0;
	//Apply mel filterbanks.
	
	for (uint32_t bin = 0; bin < (uint32_t)NUM_FBANKS; bin++) {
		mel_energy = 0;
		first_index = FBANK_FILTER_FIRST[bin];
		last_index = FBANK_FILTER_LAST[bin];
		length = last_index-first_index+1;

		// total downscaling: 4 + 31
		// Internal downscaling could be mel_pre_downscaling = 4 instead of 14b
		// Idea: custom arm_dot_prod_q31 with pre_downscaling as parameter
		// The return result is in 16.48
		// https://www.keil.com/pack/doc/CMSIS/DSP/html/group__dot__prod.html#gab15d8fa060fc85b4d948d091b7deaa11
		custom_arm_dot_prod_q31(&m->fft_abs2[first_index],
			(q31_t*) MEL_FBANK[bin], length, &mel_energy, 4);
		// meet internal downscaling, instead
		// >> 3 + 31, we do
		m->mel_energies[bin] = (q31_t) (mel_energy >> 31);
		
		//avoid log of zero
		if (m->mel_energies[bin] == 0){
			m->mel_energies[bin] = 1;
		}


		// integrated log
		m->mel_energies[bin] = (log_32(m->mel_energies[bin]) - LOG_ENERGY_CONSTANT);
	}
}

/*
	Computes mel coefficients by applying dct
	Returns Q6.2
*/
void compute_mel_coefficients(mfcc_config_t * m, q7_t * mfcc_out)
{
	// if DCT_MATRIX is normalized so dct_scaling_factor is 0,
	// if not, add
	// uint8_t dct_scaling_factor = 0;
	// if DCT_MATRIX is not normalized
	// DCT can grow up to 1/N. Here N=10
	// uint8_t dct_scaling_factor = 4;
	// Then, meeting 14 bit downscaling to avoid overflow
	// uint8_t downscaling = 29 - 14 + dct_scaling_factor;

	// grows up to 6 bits
	// downscaling = 29-14 as DCT_MATRIX is q15

	// if the NN expects inputs in Q4.3
	// downscaling = 28
	q63_t sum = 0;
	//Take DCT. (also includes cepstral lifter) Uses dot product
	for (uint32_t i = 0; i < (uint32_t)NUM_MFCC_COEFFS; i++) {
		sum = 0;
		for (uint32_t j = 0; j < (uint32_t)NUM_FBANKS; j++) {
			sum += (q63_t) ( ((q63_t)(q31_t)DCT_MATRIX[i*NUM_FBANKS+j] * (q63_t)(q31_t)m->mel_energies[j]) >> 6);
		}
		// avoid possible overflow, with -2**(7+final_scaling) and 2**(7+final_scaling)-1
		//  2147483648 = 2**(7+29-5)
		//  1073741824 = 2**(7+29-6)
		// 68719476735 = 2**(7+29)
		// avoid possible overflow, with -2**(7+29) and 2**(7+29)-1

		// Q5.2
		// if(sum >  1073741823){ sum = 1073741823; }
		// if(sum < -1073741824){ sum = -1073741824; }
		// mfcc_out[i] = (q7_t) (sum >> 23);
		
		// Q4.3
		sum = sum >> 22;
		if(sum >  127){ sum = 127; }
		if(sum < -128){ sum = -128; }
		mfcc_out[i] = (q7_t) (sum);


	}
}


/*
 * fft and shift
*/
void compute_fft(mfcc_config_t * m)
{
	//Compute FFT. FFT is stored as [real0, realN/2-1, real1, im1, real2, im2, ...].
	// output is Q10.22, not the documented one: 
	// https://github.com/ARM-software/CMSIS_5/issues/220
	// according to https://www.keil.com/pack/doc/CMSIS/DSP/html/group__RealFFT.html#gabaeab5646aeea9844e6d42ca8c73fe3a
	// output is Q9.23, and internally downscaled by 2**-8
	// but it is indeed Q10.22i and downscaled by 2**-9
	custom_arm_rfft_q31(&m->rfft, m->frame, m->fft);

	// RFFT max should be 1f when downscaled
	// multiplying by 2/512, but it is 1/512
	// to normalize abs (and therefore abs^2) x4
	// 
	// we only use half the spectrum, so only untill FRAME_LEN_PADDED
	arm_shift_q31(m->fft, 1, m->fft, FRAME_LEN_PADDED+2);
}


/*
 * module
*/
void compute_fft_abs(mfcc_config_t * m)
{
	// fft^2
	// note that numSamples states for
	// the number of complex samples in the input vector
	arm_cmplx_mag_q31(m->fft, m->fft_abs2, HALF_DIM);
}

/*
 * module^2 and shift
*/
void compute_fft_abs2(mfcc_config_t * m)
{
	// fft^2
	// note that numSamples states for
	// the number of complex samples in the input vector
	arm_cmplx_mag_squared_q31(m->fft, m->fft_abs2, HALF_DIM);
		
	// arm_cmplx_mag_squared_q31
	// downscales by 33 instead of by 31
	// The rfft abs only grows up to N/2 (input to this function 
	// as the rff is suitably scaled << 1), so we would lose 2 bits 
	arm_shift_q31(m->fft_abs2, 2, m->fft_abs2, HALF_DIM);
}

/*
	Public functions
*/
void mfcc_init(mfcc_config_t * m)
{
	// everything but fft is already innitialized or fixed in rom
	custom_arm_rfft_init_q31((& m->rfft), FRAME_LEN_PADDED, 0, 1);
}

/*
Customizable MFCC function
*/
/*
void mfcc_compute(mfcc_config_t * m, q15_t * data,
	q7_t * mfcc_out)
{

	///////////////////////
	// Audio Preprocessing
	///////////////////////

	// 1) REQUIRED if options a) or b) are selected
	//copy data to frame fft
	memcpy(m->frame, data, sizeof(q31_t) * FRAME_LEN);
	memset(&m->frame[FRAME_LEN], 0, sizeof(q31_t) * (FRAME_LEN_PADDED-FRAME_LEN));

	// 2) REQUIRED if options a) or b) are selected
	//compute DC offset, min-max, scale shift
	//required if any scaling or offset removal is done
	signal_properties_t sp = compute_signal_properties(m);

	// option a): combined DC removal, scaling, and windowing
	signal_dc_scaling_windowing(m, sp);
	// option b): combined DC removal, scaling, preemphasis, and windowing
	// signal_dc_scaling_preemphasis_windowing(m, sp);
	// option c): only windowing
	// COPY FROM DATA TO FRAME required after
	// signal_windowing(data);
	// // REQUIRED IF c)
	// arm_q15_to_q31(data, m->frame, FRAME_LEN);
	// arm_fill_q31(0, &m->frame[FRAME_LEN], FRAME_LEN_PADDED-FRAME_LEN);

	///////////////////////
	// FFT, mag, ^2
	///////////////////////
	//Compute FFT. FFT is stored as [real0, realN/2-1, real1, im1, real2, im2, ...].
	// Result is in Q9.23 if considered non-scaled
	// documentation sais FFT result has been downscaled by 8!
	custom_arm_rfft_q31(&m->rfft, m->frame, m->fft);
	// according to https://www.keil.com/pack/doc/CMSIS/DSP/html/group__RealFFT.html#gabaeab5646aeea9844e6d42ca8c73fe3a
	// output is Q9.23, and internally downscaled by 2**-8
	// but it is indeed Q10.22i and downscaled by 2**-9
	// to normalize abs (and therefore abs^2) x4
	arm_shift_q31(m->fft, 1, m->fft, 2*FRAME_LEN_PADDED);
	compute_fft_abs2(m);

	///////////////////////
	// Mel, log and DCT
	///////////////////////
	compute_mel_energies(m);
	compute_log_mel_energies(m);
	compute_mel_coefficients(m, mfcc_out);
}
*/

/*
* M0N0 MFCC function
*/
void mfcc_compute(mfcc_config_t * m, q15_t * data, q7_t * mfcc_out)
{
	///////////////////////
	// Audio Preprocessing
	///////////////////////
	arm_mult_q15(data, (q15_t*) WINDOW_FUNC, data, FRAME_LEN);
	// REQUIRED IF c)
	arm_q15_to_q31(data, m->frame, FRAME_LEN);
	arm_fill_q31(0, &m->frame[FRAME_LEN], FRAME_LEN_PADDED-FRAME_LEN);

	///////////////////////
	// FFT and mag (abs or abs^2)
	///////////////////////
	compute_fft(m);

	// compute_fft_abs(m);
	compute_fft_abs2(m);

	//////////////////////////
	// Mel & log all together
	//////////////////////////
	compute_mel_and_log_energies_together(m);

	///////////////////////
	// DCT
	///////////////////////
	compute_mel_coefficients(m, mfcc_out);

}
