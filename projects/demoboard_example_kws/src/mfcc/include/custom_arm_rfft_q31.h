
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
#ifndef CUSTOM_ARM_RFFT_Q31_H
#define CUSTOM_ARM_RFFT_Q31_H

#include <stdint.h>
//#include <malloc.h>

// #include "profile.h"
#include "arm_math.h"

#define ARMBITREVINDEXTABLE_FIXED_256_TABLE_LENGTH ((uint16_t)240)


/**
    * @brief Instance structure for the fixed-point CFFT/CIFFT function.
*/
typedef struct
{
    uint16_t fftLen;                   /**< length of the FFT. */
    const q31_t *pTwiddle;             /**< points to the Twiddle factor table. */
    const uint16_t *pBitRevTable;      /**< points to the bit reversal table. */
    uint16_t bitRevLength;             /**< bit reversal table length. */
} custom_arm_cfft_instance_q31;

/**
* @brief Instance structure for the Q31 RFFT/RIFFT function.
*/
typedef struct
{
    uint32_t fftLenReal;                        /**< length of the real FFT. */
    uint8_t ifftFlagR;                          /**< flag that selects forward (ifftFlagR=0) or inverse (ifftFlagR=1) transform. */
    uint8_t bitReverseFlagR;                    /**< flag that enables (bitReverseFlagR=1) or disables (bitReverseFlagR=0) bit reversal of output. */
    uint32_t twidCoefRModifier;                 /**< twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table. */
    q31_t *pTwiddleAReal;                       /**< points to the real twiddle factor table. */
    q31_t *pTwiddleBReal;                       /**< points to the imag twiddle factor table. */
    const custom_arm_cfft_instance_q31 *pCfft;         /**< points to the complex FFT instance. */
} custom_arm_rfft_instance_q31;

/**
 * @brief Instance structure for the Q31 CFFT/CIFFT function.
 */
typedef struct
{
    uint16_t fftLen;                 /**< length of the FFT. */
    uint8_t ifftFlag;                /**< flag that selects forward (ifftFlag=0) or inverse (ifftFlag=1) transform. */
    uint8_t bitReverseFlag;          /**< flag that enables (bitReverseFlag=1) or disables (bitReverseFlag=0) bit reversal of output. */
    q31_t *pTwiddle;                 /**< points to the twiddle factor table. */
    uint16_t *pBitRevTable;          /**< points to the bit reversal table. */
    uint16_t twidCoefModifier;       /**< twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table. */
    uint16_t bitRevFactor;           /**< bit reversal modifier that supports different size FFTs with the same bit reversal table. */
} custom_arm_cfft_radix4_instance_q31;

arm_status custom_arm_rfft_init_q31(
    custom_arm_rfft_instance_q31 * S,
    uint32_t fftLenReal,
    uint32_t ifftFlagR,
    uint32_t bitReverseFlag);

void custom_arm_rfft_q31(
    const custom_arm_rfft_instance_q31 * S,
    q31_t * pSrc,
    q31_t * pDst);


void custom_arm_cfft_q31(
    const custom_arm_cfft_instance_q31 * S,
    q31_t * p1,
    uint8_t ifftFlag,
    uint8_t bitReverseFlag);

/* Deprecated */
void custom_arm_cfft_radix4_q31(
    const custom_arm_cfft_radix4_instance_q31 * S,
    q31_t * pSrc);

/* Deprecated */
arm_status custom_arm_cfft_radix4_init_q31(
    custom_arm_cfft_radix4_instance_q31 * S,
    uint16_t fftLen,
    uint8_t ifftFlag,
    uint8_t bitReverseFlag);

arm_status custom_arm_rfft_init_q31(custom_arm_rfft_instance_q31 * S,
    uint32_t fftLenReal,
    uint32_t ifftFlagR,
    uint32_t bitReverseFlag);

void custom_arm_rfft_q31(const custom_arm_rfft_instance_q31 * S,
    q31_t * pSrc,
    q31_t * pDst);

#endif
