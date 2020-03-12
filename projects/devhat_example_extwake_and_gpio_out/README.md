# EXTWAKE and GPIO Output Example

The program displays a binary count on the four GPIOs (3 DevHat LEDs, plus one on the DevModule board below) and increments at a fixed interval. 

When updating the GPIO, it also prints the GPIO count to the console in addition to the while loop count (to show that the while loop is not being blocked). 
When EXTWAKE is pressed, it triggers an interrupt that increments an `extwake_count` counter. 
At a fixed interval (but different to the GPIO interval), the `extwake_count` is printed. 

If EXTWAKE is held down at the time of this interval, the program will wait for the key to be released, save the `extwake_count` to Shutdown RAM, and go into Shutdown Mode. 

The System is woken from shutdown mode when the EXTWAKE is pressed. 
On startup, it reads the `extwake_count` from the Shutdown RAM (if VBAT has been interrupted, it initialises `extwake_count` to 0).  

Execution continues, increment the restored `extwake_count` value. 


This example application covers:

* Using the GPIOs as outputs
* Going into (deep) shutdown mode
* Using EXTWAKE to wake from deep shutdown
* Running a callback function when EXTWAKE is pressed in active mode
* Using multiple software timers to time events using the RTC
* Detecting if VBAT has been reset when waking up
* Saving and restoring a variable in Shutdown RAM across a shutdown
* Detecting if EXTWAKE is being currently pressed

