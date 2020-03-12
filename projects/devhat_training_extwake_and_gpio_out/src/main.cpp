
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
    // TODO (GOAL 2 - remove for GOAL 3)
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
    /* TODO (GOAL 2): enable extwake interrupt with callback function
     * TIP: see sys->enable_extwake_interrupt
     */
    RTCTimer gpio_timer; // Setup GPIO software timer
    gpio_timer.set_interval_ms(1500);
    gpio_timer.reset();
    /* TODO (GOAL 3): set a timer to time the interval (10 seconds) at which to 
     * print the extwake count value to screen.
     * TIP: see the gpio_timer setup above
     */
    uint32_t gpio_count = 0;
    uint32_t loop_count = 0;
    /* TODO (GOAL 1): set gpio pins as outputs
     * TIP: see sys->gpio->* 
     */
    while (1) {
        if (gpio_timer.check_interval()) {
            gpio_timer.reset();
            if (gpio_count >= 15) {
                gpio_count = 0;
            } else {
                gpio_count++;
            }
            /* TODO (GOAL 1): set GPIO LEDs to counter value*/
            sys->log_info("GPIO: %d (loop count: %d)", gpio_count, loop_count);
        }
        /* TODO (GOAL 3): at the extwake timer interval, print the EXTWAKE count
         * TIP: see the gpio_timer above
         */
        loop_count++;
    }
    sys->log_info("Ending program");
}
