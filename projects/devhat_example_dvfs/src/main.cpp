
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

volatile bool button_pressed = false;

void button_pressed_callback(void) {
    button_pressed = true;
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_info("Button pressed callback");
}

void systick_callback(void) { 
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_info("Systick callback. Perf: %d", sys->get_perf());
    sys->gpio->write_data(~sys->gpio->read_data()); // invert GPIO
}

void setup_systick(void) {
    // in a function so that we can reset after hijacking 
    //the systick for frequency measurement
    M0N0_System* sys = M0N0_System::get_sys();
    sys->enable_systick(2000000, &systick_callback);
}

int main(void) {
    LOG_LEVEL_t log_level = DEBUG;
    M0N0_System* sys = M0N0_System::get_sys(log_level);
    sys->set_recommended_settings();
    sys->log_info("Starting DVFS Example");
    sys->print_info();  // show print_info function
    // setup GPIO
    sys->gpio->set_direction(0xF); // set all four GPIOs to output
    sys->gpio->write_data(0x0); // set all four GPIOs to OFF
    // setup EXTWAKE button
    sys->enable_extwake_interrupt(&button_pressed_callback);
    // setup Software time
    RTCTimer timer; // timer for "realtime" reference
    timer.set_interval_ms(1500);
    timer.reset();
    // setup SYSTICK
    setup_systick();
    while (1) {
        if (timer.check_interval()) {
            timer.reset();
            sys->log_info("==============");
        }
        if (sys->is_extwake()) { // displays current freq just before change
            sys->log_info("Perf: %d, Estimated frequency: %d kHz ",
                    sys->get_perf(),
                    sys->estimate_tcro());
            while(sys->is_extwake());
            setup_systick(); // must re-enable systick as estimate_tcro resets it
        }
        if (button_pressed) { // when extwake button released
            button_pressed = false;
            uint8_t current_dvfs = sys->get_perf();
            uint8_t new_dvfs = 0;
            if (current_dvfs < 15) {
                new_dvfs = current_dvfs + 1; 
            }
            sys->log_info("Old DVFS: %d, new DVFS: %d", current_dvfs, new_dvfs);
            sys->set_perf(new_dvfs);
        }
    }
    sys->log_info("Ending program"); // shouldn't reach this
}

