# Temperature Example

The M0N0 DevHat contains a [Texas Instruments TMP121](http://www.ti.com/product/TMP121) temperature sensor that can be sampled from M0N0 via SPI. 

This project provides a simple example for reading the current temperature. 


## Converting the Temperature Register to Temperature

A 16-bit value is received where:

* Bit 15 MSB is the sign bit
* Bit 2 is always 0
* Bits 0 and 1 are high impedance. 

From the datasheet, it states that the numbers increment in steps of 0.0625, which is the same as 1/16 and the value in degrees can be obtained by shifting right four times. 

Note that the TMP121 sensor is only accurate to approximately 1.5~C. 


## Further Enhancements

Note that the temperature is sampling while the chip selected is _deactivated_ (therefore consuming power while the software could be doing other things). 

The CS active-low polarity setting in the PCSM could be exploited to keep the temperature sensor CS activated (low power mode) despite it being deactivated in the software (and therefore not toggled when, say the PCSM or another SPI device is used).  


