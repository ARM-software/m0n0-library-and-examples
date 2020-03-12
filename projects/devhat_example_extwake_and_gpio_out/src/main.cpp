
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

const uint32_t kShramAddress = 29; // Any world-aligned unused shram address

uint32_t extwake_count = 0;

void extwake_func(void) {
    extwake_count++;
}

void save_to_shutdown_ram() {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_info("Saving extwake_count to Shutdown RAM");
    sys->shram->write(kShramAddress, extwake_count);
}

int main(void)
{
    LOG_LEVEL_t log_level = DEBUG;
    M0N0_System* sys = M0N0_System::get_sys(log_level);
    sys->set_recommended_settings();
    sys->log_info("Starting program...");
    if (sys->is_vbat_por()) {
        sys->log_info("VBAT was reset. Initialising...");
        // PCSM had PoR since last shutdown
        // No (reliable) data in Shutdown RAM
        // Initialise shutdown ram variable
        sys->shram->write(kShramAddress, extwake_count);
    } else {
        extwake_count = sys->shram->read(kShramAddress);
        sys->log_info("VBAT not reset. Read EXTWAKE count: %d", extwake_count);
    }
    sys->enable_extwake_interrupt(&extwake_func); // set EXTWAKE interrupt
    sys->gpio->set_direction(0xF); // Set GPIO as output
    RTCTimer gpio_timer; // Setup GPIO software timer
    gpio_timer.set_interval_ms(1500);
    gpio_timer.reset();
    RTCTimer extwake_timer; // Setup extwake print timer
    extwake_timer.set_interval_ms(10000);
    extwake_timer.reset();
    uint32_t gpio_count = 0;
    uint32_t loop_count = 0;
    while (1) {
        if (gpio_timer.check_interval()) {
            gpio_timer.reset();
            if (gpio_count >= 15) {
                gpio_count = 0;
            } else {
                gpio_count++;
            }
            sys->gpio->write_data(gpio_count);
            sys->log_info("GPIO: %d (loop count: %d)", gpio_count, loop_count);
        }
        if (extwake_timer.check_interval()) {
            extwake_timer.reset();
            sys->log_info("EXTWAKE Count: %d", extwake_count);
            if (sys->is_extwake()) {
                sys->log_info("EXTWAKE is held. Release it to go into shutdown mode...");
                while(sys->is_extwake()); // wait for extwake to be release
                // (otherwise system wake immediately)
                // Save data and go into shutddown
                save_to_shutdown_ram();
                sys->log_info("Entering Shutdown");
                sys->deep_shutdown();
            }
        }
        loop_count++;
    }
    sys->log_info("Ending program");
}
