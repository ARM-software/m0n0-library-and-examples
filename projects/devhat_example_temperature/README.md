# Temperature Example

The M0N0 DevHat contains a [Texas Instruments TMP121](http://www.ti.com/product/TMP121) temperature sensor that can be sampled from M0N0 via SPI. 

This project provides a simple example for reading the current temperature and storing the last 10 values in a circular buffer. There are four different methods; three of the four functions should be commented out leaving one that is executed. The four different examples are:

* `blocking_sw_timer`: Uses the SWTIMER (polling the RTC) to wait between samples
* `blocking_sw_timer_low_power`: The same as above but sets the DVFS to the lowest level while waiting between samples
* `blocking_inttimer_timer_low_power`: The same as above but uses a wait for interrupt (saving CPU power) and the PCSM Interrupt ("loop") timer to trigger an interrupt after the time period
* `timed_shutdown` Goes into a timed shutdown between taking samples, storing last 10 values in SHRAM and restoring on wakeup


This example application covers:

* Reading from an SPI device
* Waiting using the SWTIMER
* Changing the DVFS level
* Using the PCSM Interrupt ("loop") timer
* Using the built-in circular buffer library
* Saving and restoring the buffer to/from SHRAM
* Detecting if VBAT has been reset
* Timed shutdown mode

## Converting the Temperature Register to Temperature

A 16-bit value is received where:

* Bit 15 MSB is the sign bit
* Bit 2 is always 0
* Bits 0 and 1 are high impedance. 

From the [datasheet](http://www.ti.com/product/TMP121), it states that the numbers increment in steps of 0.0625, which is the same as 1/16 and the value in degrees can be obtained by shifting right four times. 

Note that the TMP121 sensor is only accurate to approximately 1.5~C. 

## Further Enhancements

Note that the temperature is sampling while the chip selected is _deactivated_ (therefore consuming power while the software could be doing other things). 

The CS active-low polarity setting in the PCSM could be exploited to keep the temperature sensor CS activated (low power mode) despite it being deactivated in the software (and therefore not toggled when, say the PCSM or another SPI device is used).  


