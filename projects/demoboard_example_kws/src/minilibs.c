
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
#include "minilibs.h"

void write_gpio(uint8_t val) {
    M0N0_write(GPIO_DATA_REG,val);
}

uint32_t get_sensor_data() {
    return M0N0_read(SPI_SENSOR_DATA_REG);
}

void spi_enable_adc_polling() {
    M0N0_write_bit_group(SPI_CONTROL_REG, SPI_R05_ENABLE_AUTO_SAMPLE_BIT_MASK, 1);
}

void spi_select_slave(uint8_t slave_id) {
    // Set SS2
    M0N0_write_bit_group(SPI_CONTROL_REG, SPI_R05_CHIP_SELECT_BIT_MASK, 1<<slave_id);
    M0N0_write_bit_group(SPI_CONTROL_REG, SPI_R05_ENABLE_MASK_BIT_MASK, 1);
}

void spi_deselect_slave() {
    M0N0_write_bit_group(SPI_CONTROL_REG, SPI_R05_ENABLE_MASK_BIT_MASK, 0);
    M0N0_write_bit_group(SPI_CONTROL_REG, SPI_R05_CHIP_SELECT_BIT_MASK, 0);
}

void spi_set_ss_active_low_ss2() {
    // SS2
    M0N0_write_bit_group(SPI_CONTROL_REG, SPI_R05_CS_ACTIVE_LOW_SS2_BIT_MASK, 1);
}

void clear_loop_timer() {
  // Configures SPI Slave Select (SS3)
  spi_select_slave(3);
  //stop timer loop
  M0N0_spi_write(35);            //reg_inttimer1_addr
  M0N0_spi_write(0);             //write all 0 to disable
  M0N0_spi_write(0);             //write all 0 to disable
  M0N0_spi_write(0);             //write all 0 to disable
  spi_deselect_slave(); 
}

void set_loop_timer(uint32_t interval) {
  // Configures SPI Slave Select (SS3)
  spi_select_slave(3);
  M0N0_spi_write(35);            //reg_inttimer0_addr
  M0N0_spi_write(((interval-1)>>16)&0xff);
  M0N0_spi_write(((interval-1)>>8)&0xff);
  M0N0_spi_write((interval-1)&0xff);
  spi_deselect_slave();
}

