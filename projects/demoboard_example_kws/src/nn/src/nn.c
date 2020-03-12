
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
// Keyword spotting DNN implementation
// #include "m0n0_printf.h"
#include "arm_nnfunctions.h"

#include "weights.h"
#include "nn.h"

/* Network Structure

		8 inputs per time step
		(total_ 16 time steps (from 62.5ms windowsing))
		|
		L_1 : Innerproduct (weights: 128x128)
		|
		L_2 : Innerproduct (weights: 128x128)
		|
		L_3 : Innerproduct (weights: 128x12)
		|
		12 outputs

*/

// Constants from rom
static const q7_t l_0_wt[L_0_W_DIM] = L_0_W;
static const q7_t l_0_bias[L_0_OUT_DIM] = L_0_B;
static const q7_t l_1_wt[L_1_W_DIM] = L_1_W;
static const q7_t l_1_bias[L_1_OUT_DIM] = L_1_B;
static const q7_t l_2_wt[L_2_W_DIM] = L_2_W;
static const q7_t l_2_bias[OUT_DIM] = L_2_B;


// buffers to ram
q7_t l_0_out[L_0_OUT_DIM];
q7_t l_1_out[L_1_OUT_DIM];
q15_t vec_buffer[2*L_0_OUT_DIM];


void clear_nn_buffers()
{
	// set to non-zero val_ues
	memset(l_0_out, 1, sizeof(l_0_out));
	memset(l_1_out, 2, sizeof(l_1_out));
	memset(vec_buffer, 4, sizeof(vec_buffer));
}

void run_nn(q7_t* in_data, q7_t* out_data)
{
	// from CMSIS_NN:

	// * @brief Q7 basic fully-connected layer function
	// * @param[in]       pV          pointer to input vector
	// * @param[in]       pM          pointer to matrix weights
	// * @param[in]       dim_vec     length of the vector
	// * @param[in]       num_of_rows number of rows in weight matrix
	// * @param[in]       bias_shift  amount of left-shift for bias
	// * @param[in]       out_shift   amount of right-shift for output
	// * @param[in]       bias        pointer to bias
	// * @param[in,out]   pOut        pointer to output vector
	// * @param[in,out]   vec_buffer  pointer to buffer space for input
	// * @return     The function returns <code>ARM_MATH_SUCCESS</code>
	
	// Run all layers
	// L1
	arm_fully_connected_q7(in_data, l_0_wt, IN_DIM, L_0_OUT_DIM, 3, 7, l_0_bias, l_0_out, vec_buffer);
	arm_relu_q7(l_0_out, L_0_OUT_DIM);
	// L_2
	arm_fully_connected_q7(l_0_out, l_1_wt, L_0_OUT_DIM, L_1_OUT_DIM, 3, 7, l_1_bias, l_1_out, vec_buffer);
	arm_relu_q7(l_1_out, L_1_OUT_DIM);
	// output layer
	// L_4
	arm_fully_connected_q7(l_1_out, l_2_wt, L_1_OUT_DIM, OUT_DIM, 1, 8, l_2_bias, out_data, vec_buffer);
}
