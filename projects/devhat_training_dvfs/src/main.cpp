
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
    // in a function so that we can reset after "hijacking"
    //the systick for frequency measurement
    /* TODO TASK enable the systick with 2000000 cycles and 
     * setting the systick_callback as the callback function
     */
}

int main(void) {
    LOG_LEVEL_t log_level = DEBUG;
    M0N0_System* sys = M0N0_System::get_sys(log_level);
    sys->set_recommended_settings();
    sys->log_info("Starting DVFS Example");
    // setup GPIO
    sys->gpio->set_direction(0xF); // set all four GPIOs to output
    sys->gpio->write_data(0x0); // set all four GPIOs to OFF
    // setup EXTWAKE button
    /* TODO Task: set EXTWAKE interrupt and callback function
     * Tip: use the enable_extwake_interrupt function and pass it 
     * a pointer to the button_pressed_callback function
     */
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
            /* TODO TASK: print the current perf and the estimated TCRO
             * frequency. TIP: see the `sys->get_perf' function and 
             * sys->estimate_tcro function
             */
            while(sys->is_extwake()); // wait for EXTWAKE to be released
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
            /* TODO Task: set the new perf*/
        }
    }
    sys->log_info("Ending program"); // shouldn't reach this
}

