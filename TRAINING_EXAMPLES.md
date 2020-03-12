# Training Examples

Several example programs have been written to demonstrate M0N0 System features. 

More details about each example are in README files in their corresponding directories:

* [devhat_example_extwake_and_gpio_out](projects/devhat_example_extwake_and_gpio_out/README.md) The LEDs count. When EXTWAKE is pressed, a counter is incremented (value printed via STDOUT at an interval). When EXTWAKE is held down across the interval, the system goes into shutdown. Pressing EXTWAKE wakes the system up and it should remember the count. This example demonstrates:

    * GPIO outputs;
    * using the EXTWAKE button interrupt in software;
    * (deep) shutdown mode
    * saving to SHRAM and restoring; and
    * using the RTC for non-blocking task timing (via the SWTIMER library function). 
* [devhat_example_temperature](projects/devhat_example_temperature/README.md) Records the temperature (and RTC timestamp) at regular intervals and saves the last 10 values (it prints to STDOUT). There are four different modes, from inefficient (power-wise) software polling of the RTC, to using the timed-shutdown mode. It demonstrates:

    * Using the RTC to wait
    * Using the PCSM Interval timer and interrupt
    * Using timed-shutdown mode
    * Using the M0N0 library circular buffer
    * Saving and restoring data between shutdowns

* [devhat_example_dvfs](projects/devhat_example_dvfs/README.md) The LEDs flash at a rate proportional to the DVFS level (using the SysTick timer). Releasing EXTWAKE increments the frequency (it eventually wraps back around to the minimum value). Holding the EXTWAKE pauses the program before the DVFS change, and displays current frequency, as estimated using the RTC and SysTick ratio. Demonstrates:

    * Using EXTWAKE with a software interrupt
    * Using GPIOs as outputs
    * Using a software timer with the RTC
    * Using the SysTick timer and interrupt



