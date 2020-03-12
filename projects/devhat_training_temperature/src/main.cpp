
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

#include "m0n0.h"
const uint32_t kDataLength = 10; // length of the data
uint32_t temperature_array[kDataLength]; // array for storing temperature 

void setup_temperature_sensor() {
    // setting up SPI
    // In this specific example, this function is redundant
    // temperature sensor is connected to SS0
    // set SPI control reg to active low (same as default value)
    M0N0_System* sys = M0N0_System::get_sys();
    sys->spi->write(SPI_CONTROL_REG, SPI_R05_CS_ACTIVE_LOW_SS0_BIT_MASK,1);
}

// sys is the M0N0 System instance
// timer is the temperature timer (how long to keep CS active
int16_t read_temperature() {
    SPI_SS_t temperature_ss = SS0;
    SPI_SS_t deselect_ss = DESELECT;
    M0N0_System* sys = M0N0_System::get_sys();
    // the temperature is sampled all the while the CS is DEACTIVATED
    sys->spi->set_slave(temperature_ss);
    uint8_t byte0 = sys->spi->write_byte(0);
    uint8_t byte1 = sys->spi->write_byte(0);
    sys->spi->set_slave(deselect_ss);
    uint16_t temp_reg = (((uint16_t)byte0 << 8) | byte1);
    uint8_t temp_sign = temp_reg & (1<<15);
    int16_t temperature = (int16_t)(((temp_reg) & ~(1<<15))>>3) >> 4;
    if (temp_sign) {
        temperature = 0-temperature;
    }
    sys->log_info("Temperature: %d", temperature);
    return temperature;
}

int main(void) {
    LOG_LEVEL_t log_level = DEBUG;
    M0N0_System* sys = M0N0_System::get_sys(log_level);
    sys->set_recommended_settings();
    sys->log_info("Starting temperature example"); 
    uint32_t interval_ms = 2000;
     // Detect whether this is the first run or not
    // I.e. whether there is existing data to be restored
    if (!sys->is_vbat_por()) {
        // Restore data from shram
        sys->log_info("No VBAT PoR - any data in SHRAM is intact");
    } else {
        // initialise data
        sys->log_info("VBAT PoR - cold start, no data in SHRAM");
    }
    sys->log_info("Setting up sensor");
    setup_temperature_sensor();
    sys->log_info("Running temperature example");
    while(1) {
        // First, use read_temperature and log_info to print the temperature.
        // A delay must be put between reads - first try sys->sleep_us or the 
        // RTCTimer object (see README.md and the devhat_example_temperature
        // project. 
        sys->log_info("TODO: read and report temperature");
        sys->sleep_ms(interval_ms*1000); // try using RTCTimer instead of this
    }
    sys->log_info("Ending program"); // shouldn't reach this
}


