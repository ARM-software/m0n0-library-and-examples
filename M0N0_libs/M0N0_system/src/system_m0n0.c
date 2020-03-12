/**************************************************************************//**
 * @file		 system_ARMCM33.c
 * @brief		CMSIS Device System Source File for
 *					 ARMCM33 Device
 * @version	V5.3.1
 * @date		 09. July 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined (ARMCM33)
	#include "ARMCM33.h"
#elif defined (ARMCM33_TZ)
	#include "ARMCM33_TZ.h"
	#if defined (__ARM_FEATURE_CMSE) &&	(__ARM_FEATURE_CMSE == 3U)
		#include "partition_ARMCM33.h"
	#endif
#elif defined (ARMCM33_DSP_FP)
	#include "ARMCM33_DSP_FP.h"
#elif defined (ARMCM33_DSP_FP_TZ)
	#include "ARMCM33_DSP_FP_TZ.h"

	#if defined (__ARM_FEATURE_CMSE) &&	(__ARM_FEATURE_CMSE == 3U)
		#include "partition_ARMCM33.h"
	#endif
#else
	#error device not specified!
#endif

/*----------------------------------------------------------------------------
	Define clocks
 *----------------------------------------------------------------------------*/
#define XTAL	(50000000UL)	 /* Oscillator frequency */
#define	SYSTEM_CLOCK	(XTAL / 2U)


/*----------------------------------------------------------------------------
	Externals
 *----------------------------------------------------------------------------*/
//#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
//	extern uint32_t __Vectors;
//#endif

/*----------------------------------------------------------------------------
	System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;	/* System Core Clock Frequency */


/*----------------------------------------------------------------------------
	System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
	SystemCoreClock = SYSTEM_CLOCK;
}

/*----------------------------------------------------------------------------
	System initialization function
 *----------------------------------------------------------------------------*/
void SystemInit (void)
{

#if defined (__FPU_USED) && (__FPU_USED == 1U)
	#error m0n0 does not support FPU
	SCB->CPACR |= ((3U << 10U*2U) |	 /* enable CP10 Full Access */
			(3U << 11U*2U)); /* enable CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
	SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
	TZ_SAU_Setup();
#endif
	SystemCoreClock = SYSTEM_CLOCK;

	asm(
	
	"MOVS	r0, #0x20\n\t"
	"LSLS	r0, r0, #8\n\t"
	"MOVS	r1, #0x0\n\t"
	"ORRS	r0, r1\n\t"
	"LSLS	r0, r0, #8\n\t"
	"MOVS	r1, #0x3f\n\t"
	"ORRS	r0, r1\n\t"
	"LSLS	r0, r0, #8\n\t"
	"MOVS	r1, #0xf0\n\t"
	"ORRS	r0, r1\n\t"
	// Write 0 to location 0x20003fe8
	"MOVS	r2, #0x0\n\t"
	"STR	 r2, [r0]\n\t"
	// Initialise all registers to zero
	// Add PLL initialisation etc.
	"MOVS	r0, #0x0\n\t"
	"MOV	 r1, r0\n\t"
	"MOV	 r2, r0\n\t"
	"MOV	 r3, r0\n\t"
	"MOV	 r4, r0\n\t"
	"MOV	 r5, r0\n\t"
	"MOV	 r6, r0\n\t"
	"MOV	 r7, r0\n\t"
	"MOV	 r8, r0\n\t"
	"MOV	 r9, r0\n\t"
	"MOV	 r10,r0\n\t"
	"MOV	 r11,r0\n\t"
	"MOV	 r12,r0"
	);
}
