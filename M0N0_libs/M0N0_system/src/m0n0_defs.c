
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
#include "m0n0_defs.h"

uint32_t M0N0_read(uint32_t address) {
    volatile uint32_t val = 0;
    volatile uint32_t *a = (uint32_t*) address;
    val = *a;
    return val;
}

uint32_t M0N0_read_bit_group(
        uint32_t address,
        uint32_t mask) {
    uint8_t shift = mask_to_shift(mask);
    return M0N0_read_mask_and_shift(
        address,
        shift,
        mask
    );
}

uint32_t M0N0_read_mask_and_shift(
        uint32_t address,
        uint32_t shift,
        uint32_t mask) {
    volatile uint32_t val = M0N0_read(address);
    return (val & mask) >> shift;
}

void M0N0_write(uint32_t address, uint32_t data) {
    *((__IO uint32_t *)(address)) = data;
}

void M0N0_write_bit_group(
        uint32_t address,
        uint32_t mask,
        uint32_t data) {
    uint8_t shift = mask_to_shift(mask);
    M0N0_write_mask_and_shift(
        address,
        shift,
        mask,
        data
    );
}

void M0N0_write_mask_and_shift(
        uint32_t address,
        uint32_t shift,
        uint32_t mask,
        uint32_t data) {
    uint32_t reg = M0N0_read(address) & ~(mask);
    M0N0_write(address,reg | ((data << shift) & mask));
}

uint8_t mask_to_shift(uint32_t mask) {
    static const uint32_t lookup[] = {
      32, 0, 1, 26, 2, 23, 27, 0, 3, 16, 24, 30, 28, 11, 0, 13, 4,
      7, 17, 0, 25, 22, 31, 15, 29, 10, 12, 6, 0, 21, 14, 9, 5,
      20, 8, 19, 18
    };
    return (uint32_t)lookup[(-mask & mask)%37];
}

char M0N0_read_stdin(void) {
    while ((M0N0_read(STDIN_STATUS_REG) & 0x1) != 0) {
        // wait
    }
    char c = (char)(M0N0_read(STDIN_RDATA_REG));
    return c; // read char
}

uint8_t M0N0_is_deve(void) {
    return M0N0_read_bit_group(
            STATUS_STATUS_7_REG,
            STATUS_R07_DEVE_CORE_BIT_MASK);
}

void M0N0_write_stdout(uint8_t data) {
    //exectb_mcu_char_write(data);
    //return;
    while (M0N0_read_mask_and_shift(
            STDOUT_STATUS_REG,
            STDOUT_R02_TXF_BIT_SHIFT,
            STDOUT_R02_TXF_BIT_MASK) != 0);
    while ((STDOUT->STAT & 0x2) != 0); // wait until space in fifo
    M0N0_write_mask_and_shift(
            STDOUT_WDATA_REG,
            STDOUT_WRITE_CHAR_BIT_SHIFT,
            STDOUT_WRITE_CHAR_BIT_MASK,
             data); // Write char
    return;
}


// Routine to write a char - specific to ADP STDOUT FIFO
void exectb_mcu_char_write(int ch)
{
    while ((STDOUT->STAT & 0x2) != 0); // wait until space in fifo
    STDOUT->WDATA = ch; // Write char
    return;
}

// Routine to read a char - specific to ADP STDIN FIFO
int exectb_mcu_char_read(void)
{
    while ((STDIN->STAT & 0x1) != 0); // wait until fifo non-empty
    return (int)(STDIN->RDATA); // read char
}

uint8_t M0N0_spi_write(uint8_t data) {
    M0N0_write(SPI_DATA_WRITE_REG, data);
    M0N0_write(SPI_COMMAND_REG, 1);
    __NOP();
    __NOP();
    while (M0N0_read(SPI_STATUS_REG) == 1); // Wait while busy
    return (uint8_t)M0N0_read(SPI_DATA_READ_REG);
}

