
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
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/**
@file
@brief C Interrupt Handlers (shared between projects by default)
*/

/** Counter that is incremented inside the SysTick_Handler
 */
volatile unsigned int systick_flag;
/** Counter that is incremented inside the Interrupt0_Handler (GPIO)
 */
volatile unsigned int interrupt0_flag;
/** Counter that is incremented inside the Interrupt1_Handler (SPI autosample)
 */
volatile unsigned int interrupt1_flag;
/** Counter that is incremented inside the Interrupt5_Handler (PCSM timer)
 */
volatile unsigned int interrupt5_flag;
/** Counter that is incremented inside the Interrupt6_Handler (EXTWAKE)
 */
volatile unsigned int interrupt6_flag;

/** Default Handler
 */
void Default_Handler(void);
/** HardFault Handler
 */
void HardFault_Handler(void);
/** MemoryManage Handler
 */
void MemManage_Handler(void);
/** BusFault Handler
 */
void BusFault_Handler(void) ;
/** UsageFault Handler
 */
void UsageFault_Handler(void);
/** SysTick Handler
 */
void SysTick_Handler(void);
/** Interrupt0 (GPIO) Handler
 */
void Interrupt0_Handler(void);
/** Interrupt1 (SPI autosample complete) Handler
 */
void Interrupt1_Handler(void);
/** Interrupt5 (PCSM "loop" timer) Handler
 */
void Interrupt5_Handler(void);
/** Interrupt5 (EXTWAKE input) Handler
 */
void Interrupt6_Handler(void);

#endif // INTERRUPTS_H
