
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
#include "m0n0_printf.h"
#include "m0n0_defs.h"
#include "minilibs.h"

#include "kws.h"

// Main method
int main()
{
	// Reset verbosity if VDEV domain is powered down.
	uint8_t verbose = 1;
	run_kws(verbose);
	write_gpio(0xF);
}


//
// Handlers
//
// Uncomment if used from a main that needs to choose which handler call


uint32_t is_deve(void) {
    return M0N0_read_bit_group(
            STATUS_STATUS_7_REG,
            STATUS_R07_DEVE_CORE_BIT_MASK);
}

void HardFault_Handler(void) {
    if (is_deve()) { // if DEVE mode enabled
        m0n0_printf("HardFault_Handler()\n");
        m0n0_printf("CFSR: 0x%X, HFSE: 0x%X, MMFAR: 0x%X, BFAR: 0x%X\n",
                SCB->CFSR,
                SCB->HFSR,
                SCB->MMFAR,
                SCB->BFAR
                );
    }
    while(1);
}

void Default_Handler(void) {
    if (is_deve()) { // if DEVE mode enabled
        m0n0_printf("Default_Handler()\n");
    }
    while(1);
}


void MemManage_Handler(void) {
    if (is_deve()) { // if DEVE mode enabled
        m0n0_printf("MemManage_Handler()\n");
    }
    while(1);
}

void BusFault_Handler(void) {
    m0n0_printf("BusFault_Handler()\n");
    while(1);
}

void UsageFault_Handler(void) {
    m0n0_printf("UsageFault_Handler()\n");
    while(1);
}

void SysTick_Handler(void) {
}

// GPIO Interrupt
void Interrupt0_Handler(void) {
}


// PCSM IntTimer Interrupt
void Interrupt5_Handler(void) {
}

// EXTAKE Interrupt
void Interrupt6_Handler(void) {
}



