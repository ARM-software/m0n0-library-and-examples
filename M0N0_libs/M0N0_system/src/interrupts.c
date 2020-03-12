
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
#include "interrupts.h"
#include "m0n0_printf.h"
#include "m0n0_defs.h"
#include "system_ARMCM33.h" // exectb_mcu System
/////////////////////////
// Handlers
////////////////////////

// Implemented in M0N0.cpp:
void hand_extwake();
void hand_systick();
void hand_pcsm_timer();
void hand_autosample();

void HardFault_Handler(void) {
    if (M0N0_is_deve()) { // if DEVE mode enabled
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
    if (M0N0_is_deve()) { // if DEVE mode enabled
        m0n0_printf("Default_Handler()\n");
    }
    while(1);
}


void MemManage_Handler(void) {
    if (M0N0_is_deve()) { // if DEVE mode enabled
        m0n0_printf("MemManage_Handler()\n");
    }
    while(1);
}

void BusFault_Handler(void) {
    if (M0N0_is_deve()) { // if DEVE mode enabled
        m0n0_printf("BusFault_Handler()\n");
    }
    while(1);
}

void UsageFault_Handler(void) {
    if (M0N0_is_deve()) { // if DEVE mode enabled
        m0n0_printf("UsageFault_Handler()\n");
    }
    while(1);
}

void SysTick_Handler(void) {
    systick_flag += 1;
    hand_systick();
    if (M0N0_is_deve()) { // if DEVE mode enabled
        m0n0_printf("SysTick_Handler()\n");
    }
}

// GPIO Interrupt
void Interrupt0_Handler(void) {
    interrupt0_flag +=1;
    // This is GPIO0 interrupt
    if (M0N0_is_deve()) {
        m0n0_printf("IRQGPIO\n");
    }
}

// Autosample Interrupt
void Interrupt1_Handler(void) {
    interrupt1_flag +=1;
    hand_autosample(); // NOTE: must be before printf due to PCSM timing
    //if (M0N0_is_deve()) {
    //    m0n0_printf("Autosample\n");
    //}
}

// PCSM IntTimer Interrupt
void Interrupt5_Handler(void) {
    interrupt5_flag += 1;
    hand_pcsm_timer();
    if (M0N0_is_deve()) {
        m0n0_printf("PCSMINTTIMERIRQ");
    }
}

// EXTAKE Interrupt
void Interrupt6_Handler(void) {
    interrupt6_flag += 1;
    hand_extwake();
    if (M0N0_is_deve()) {
        m0n0_printf("IRQEXTWAKE\n");
    }
}



