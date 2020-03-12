# DVFS Example

This example shows the DVFS functionality working. 

All four LEDs simultaneously blink after 2000000 TCRO cycles (using the Cortex-M33 SysTick timer). When EXTWAKE is release, the DVFS level increases (when at the maximum level, it wraps back around to the lowest level). While the EXTWAKE button is being held, it prints the current frequency, as estimated using the SysTick counter and the RTC. 


This example application covers:

* Using the SysTick timer to generate an interrupt after N cycles
* Using the built-in library function for estimating the current frequency
* Triggering a software interrupt when EXTWAKE is pressed
* Using the SWTIMER to time non-blocking waits
* Using the GPIOs as outputs
* Printing important chip status information


Example of the reported estimated frequencies collated together:
```
INFO:  Perf: 0, Estimated frequency: 984 kHz
INFO:  Perf: 1, Estimated frequency: 1718 kHz
INFO:  Perf: 2, Estimated frequency: 2434 kHz
INFO:  Perf: 3, Estimated frequency: 3508 kHz
INFO:  Perf: 4, Estimated frequency: 4099 kHz
INFO:  Perf: 5, Estimated frequency: 4242 kHz
INFO:  Perf: 6, Estimated frequency: 5817 kHz
INFO:  Perf: 7, Estimated frequency: 6480 kHz
INFO:  Perf: 8, Estimated frequency: 7178 kHz
INFO:  Perf: 9, Estimated frequency: 8628 kHz
INFO:  Perf: 10, Estimated frequency: 10185 kHz
INFO:  Perf: 11, Estimated frequency: 14731 kHz
INFO:  Perf: 12, Estimated frequency: 16056 kHz
INFO:  Perf: 13, Estimated frequency: 20907 kHz
INFO:  Perf: 14, Estimated frequency: 27584 kHz
INFO:  Perf: 15, Estimated frequency: 39237 kHz
```

