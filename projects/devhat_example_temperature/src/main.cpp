
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
#include "m0n0_printf.h"

// Prototypes
void buffer_filled_callback();
void buffer_empty_callback();
void buffer_read_error_callback();

const uint32_t kDataLength = 10;
uint32_t time_array[kDataLength]; // array for storing sample times
uint32_t temperature_array[kDataLength]; // array for storing temperature 
CircBuffer time_buf;
CircBuffer temperature_buf;

volatile bool pcsm_timer_occured = false; // for example C only

void buffer_filled_callback() {
    M0N0_System::error("Callback: buffer full");
}

void buffer_empty_callback() {
    M0N0_System::error("Callback: buffer empty");
}

void buffer_read_error_callback() {
    M0N0_System::error("Callback: buffer read error");
}

void setup_temperature_sensor() {
    // setting up SPI
    // In this specific example, this function is rather redundant
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

void measure_and_store() {
    M0N0_System* sys = M0N0_System::get_sys();
    // measure:
    uint32_t time_ms = sys->get_rtc_us()/1000;
    int16_t temperature = read_temperature();
    // store:
    time_buf.append(time_ms);
    temperature_buf.append(temperature);
    // print:
    sys->log_info("Time (ms):    ");
    time_buf.print_array();
    sys->log_info("Temperature: ");
    temperature_buf.print_array();
}

void blocking_sw_timer(uint32_t interval_ms) {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->print("===========================\n");
    sys->log_info("Example A: SW blocking (basic)");
    sys->print("===========================\n");
    RTCTimer loop_timer; 
    loop_timer.set_interval_ms(interval_ms);
    while (1) {
        loop_timer.wait(); // blocks
        measure_and_store();
    }
}

void blocking_sw_timer_low_power(uint32_t interval_ms) {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->print("===========================\n");
    sys->log_info("Example B: SW blocking LP");
    sys->print("===========================\n");
    RTCTimer loop_timer; 
    loop_timer.set_interval_ms(interval_ms);
    while (1) {
        loop_timer.wait_lp(); // low-power blocking
        measure_and_store();
    }
}

void pcsm_inttimer_callback(void) {
    pcsm_timer_occured = true;
    // in practice, expensive functions such as STDOUT should be avoided:
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_info("Callback example"); 
}

void blocking_inttimer_timer_low_power(uint32_t interval_ms) { // i.e. using WFI and loop timer
    M0N0_System* sys = M0N0_System::get_sys();
    sys->print("===========================\n");
    sys->log_info("Example C: SW blocking LP + WFI");
    sys->print("===========================\n");
    sys->enable_pcsm_interrupt_timer_ms(interval_ms, &pcsm_inttimer_callback);
    uint8_t orig_perf = sys->get_perf(); // save perf
    pcsm_timer_occured = true;
    while (1) {
        if (pcsm_timer_occured) {
            sys->set_perf(orig_perf); // restore orignal perf
            sys->log_info("Measure and Store");
            measure_and_store();
            sys->set_perf(0); // set to lowest perf
            pcsm_timer_occured = false;
            sys->clear_cpu_deepsleep(); // shoudn't be set anyway
            __WFI();
        }
    }
}

void timed_shutdown(uint32_t interval_ms) {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->print("===========================\n");
    sys->log_info("Example D: Timed shutdown");
    sys->print("===========================\n");
    measure_and_store();
    time_buf.store_to_shram(); // save to Shutdown RAM
    temperature_buf.store_to_shram(); // save to Shutdown RAM
    sys->timed_shutdown_ms(interval_ms);
}

int main(void) {
    LOG_LEVEL_t log_level = DEBUG;
    M0N0_System* sys = M0N0_System::get_sys(log_level);
    sys->set_recommended_settings();
    sys->log_info("Starting temperature example"); 
    time_buf = CircBuffer(
        time_array, // pass the array
        kDataLength, // pass the length of the array
        0, // the SHRAM address to save to before shutdown
        true, // true=old data is overwritten when full
        &buffer_filled_callback, // callback when full (NA if overwriting)
        &buffer_empty_callback, // callback when removing from empty buf
        &buffer_read_error_callback); // callback when a read error
    temperature_buf = CircBuffer(
        temperature_array, 
        kDataLength,
        0+(kDataLength*4)+(2*4), // Store after the time_buf in SHRAM
        true,
        &buffer_filled_callback,
        &buffer_empty_callback,
        &buffer_read_error_callback);
    uint32_t interval_ms = 2000;
     // Detect whether this is the first run or not
    // I.e. whether there is existing data to be restored
    if (!sys->is_vbat_por()) {
        // Restore data from shram
        sys->log_info("Restoring");
        time_buf.load_from_shram();
        temperature_buf.load_from_shram();
    } else {
        // initialise data
        sys->log_info("Initialising");
        time_buf.store_to_shram();
        temperature_buf.store_to_shram();
    }
    sys->log_info("Setting up sensor");
    setup_temperature_sensor();
    sys->log_info("Running temperature example");
    /*
     * Uncomment to leave the remaining timer here:
     */
    blocking_sw_timer(interval_ms);
    //blocking_sw_timer_low_power(interval_ms);
    //blocking_inttimer_timer_low_power(interval_ms);
    //timed_shutdown(interval_ms);
    /*
     * ^^ Uncomment to leave the remaining timer above ^^
     */
    sys->log_info("Ending program"); // shouldn't reach this
}


