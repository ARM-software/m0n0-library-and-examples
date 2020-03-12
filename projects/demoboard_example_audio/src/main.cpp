
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

// Function prototypes:
// status:
void set_state_idle(void);
void set_state_acquiring(void);
void set_state_finished(void);
// callbacks:
void buffer_read_error_callback(void);
void extwake_callback(void);
void audio_callback(void);
// audio sampling:
void enable_uphone_sampling(uint32_t sample_interval_rtc_ticks);
void disable_uphone_sampling();

const uint32_t kDataLength = 2048;
uint32_t audio_array[kDataLength];
CircBuffer audio_buf(
            audio_array, // pass the array
            kDataLength, // pass the length of the array
            0, // the SHRAM address to save to before shutdown
            false, // true=old data is overwritten when full
            NULL, // callback when full (NA if overwriting)
            NULL, // callback when removing from empty buf
            &buffer_read_error_callback); // callback when a read error
volatile bool has_finished = false;
uint32_t interval_rtc = 4 ;

// for timing audio sample
RTCTimer audio_timer;
uint32_t audio_recording_rtc_cycles = 0;

void set_state_idle() {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_info("IDLE");
}

void set_state_acquiring() {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_info("ACQ");
}

void set_state_finished() {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_info("FINISHED");
}

void buffer_read_error_callback() {
    M0N0_System::error("Callback: buffer read error");
}

void extwake_callback(void) {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_info("Extwake pressed");
    sys->disable_extwake_interrupt();
    set_state_acquiring();
    enable_uphone_sampling(interval_rtc);
}

void audio_callback(void) {
    M0N0_System* sys = M0N0_System::get_sys();
    uint32_t audio_frame = 0;
    audio_frame = sys->spi->read(SPI_SENSOR_DATA_REG);
    //sys->log_debug("Audio sample: 0x%X", audio_frame);
    audio_buf.append(audio_frame);
    if (audio_buf.is_full()) {
        audio_recording_rtc_cycles = audio_timer.get_cycles();
        sys->disable_autosampling();
        has_finished = true;
    }
}

void enable_uphone_sampling(uint32_t sample_interval_rtc_ticks) {
    // setting up SPI
    // In this specific example, this function is rather redundant
    // temperature sensor is connected to SS0
    // set SPI control reg to active low (same as default value)
    M0N0_System* sys = M0N0_System::get_sys();
    audio_buf.reset();
    sys->spi->write(SPI_CONTROL_REG, SPI_R05_CS_ACTIVE_LOW_SS2_BIT_MASK,1);
    sys->enable_autosampling_rtc_ticks(
            sample_interval_rtc_ticks,
            &audio_callback);
    audio_timer.reset();
}

int main(void) {
    LOG_LEVEL_t log_level = DEBUG;
    M0N0_System* sys = M0N0_System::get_sys(log_level);
    sys->set_recommended_settings();
    set_state_idle();
    sys->log_info("Starting audio example"); 
    // instantiate a circular buffer for storing the audio samples 
    audio_buf.print();
    // Detect whether this is the first run or not
    // I.e. whether there is existing data to be restored
    if (!sys->is_vbat_por()) {
        // Restore data from shram
        sys->log_info("Restoring");
        audio_buf.load_from_shram();
    } else {
        // initialise data
        sys->log_info("Initialising");
        audio_buf.store_to_shram();
    }
    sys->log_info("Setting up uphone");
    sys->enable_extwake_interrupt(&extwake_callback);
    sys->log_info("Running audio example");
    while (1) {
        if (has_finished) {
            set_state_finished();
            sys->adp_tx_start("demoboard_audio");
            sys->print("\nsample_freq_hz : 8000");
            sys->print("\nperiod_rtc_ticks : %d", interval_rtc);
            sys->print("\nrecording_rtc_cycles : %d", audio_recording_rtc_cycles);
            sys->adp_tx_end_of_params();
            audio_buf.send_via_adp();
            sys->adp_tx_end();
            has_finished = false;
            sys->enable_extwake_interrupt(&extwake_callback);
        }
    }
    sys->log_info("Ending program"); // shouldn't reach this
}

