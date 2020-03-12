# Temperature Training Example

This training example is for use with the DevBoard with the DevHat module. 

The M0N0 DevHat contains a [Texas Instruments TMP121](http://www.ti.com/product/TMP121) temperature sensor that can be sampled from M0N0 via SPI. 

The functions for setting up the SPI temperature sensor (`setup_temperature_sensor`) and reading the SPI temperature sensor `read_temperature` are provided already. 

An example of reading the temperature using four different approaches is provided in the `devhat_example_temperature` project. 

The end-goal of this training example is to have the M0N0 system sampling the built-in temperature sensor at a specific interval and printing the current temperature value. 
The first few goals focus on reading the temperature RTC time in active mode while later goals incorporate lower power techniques of sampling the temperature (i.e. using the Timed-Shutdown Feature). 

Training goals:

1. Read the temperature periodically and print to the screen using `read_temperature`, `sys->log_info` and `sys->sleep_ms`. 
2. Use the `RTCTimer` for timing the sample period (tip: see the `blocking_sw_timer` function in the `devhat_example_temperature` project). 
3. Print the RTC time value along with the temperature (tip: use the `get_rtc_us` function)
4. Put the CPU into a lower power mode using WFI and get the PCSM timer to wake the CPU after a time period (tip: see the `blocking_inttimer_timer_low_power` function of the `devhat_example_temperature` project). 
5. Store the last 10 recorded RTC and temperature values in a circular/ring buffer (i.e. use `CircBuffer` object). 
6. Use a "timed shutdown" to sleep between samples, which has the lowest power consumption (tip: see the `timed_shutdown` function of the `devhat_example_temperature`). 
7. Use Shutdown RAM to store the historic values between timed shutdowns.

