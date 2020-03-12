# EXTWAKE and GPIO Output Example

The aim of this training exercise is to create a program that displays a binary count on the GPIOs (3 DevHat LEDs, plus one on the DevModule board below) and increments at a fixed interval. This interval must not block execution because, at a different interval, a standard output (printf) message should be printed showing the number of times the EXTWAKE button has been pressed. 

As an extension, if the EXTWAKE button is held down, the system should go into deep shutdown and remember the EXTWAKE count when woken up using the EXTWAKE button. 

Implement the program in the `src/main.cpp` file in this project (`devhat_training_extwake_and_gpio_out`), where there is already a basic starting point that can be compiled and run.

Work through the goals, one-by-one below. 

## Program Overview

When updating the GPIO, it also prints the GPIO count to the console in addition to the while loop count (to show that the while loop is not being blocked). 
When EXTWAKE is pressed, it triggers an interrupt that increments an `extwake_count` counter. 
At a fixed interval (but different to the GPIO interval), the `extwake_count` is printed. 

If EXTWAKE is held down at the time of this interval, the program will wait for the key to be released, save the `extwake_count` to Shutdown RAM, and go into Shutdown Mode. 

The System is woken from shutdown mode when the EXTWAKE is pressed. 
On startup, it reads the `extwake_count` from the Shutdown RAM (if VBAT has been interrupted, it initialises `extwake_count` to 0).  

Execution continues, increment the restored `extwake_count` value. 


There is an example solution at `projects/devhat_example_extwake_and_gpio_out/`. 

## Goals:

1. Setup the GPIO as an output and make the `gpio_count` appear on the LEDs
2. Setup the EXTWAKE interrupt callback function which increments the `extwake_count` - temporarily print from the callback function itself to check that it is being called
3. Remove the printf from the callback function and setup the extwake print timer to print the EXTWAKE count to screen every 10 seconds
4. (extension) Make the system go into shutdown mode if EXTWAKE is held across the EXTWAKE print (Tip: see the `devhat_example_extwake_and_gpio_out`)
5. (extension) Save and restore the `extwake_count` various between interrupts (Tip: see the `devhat_example_extwake_and_gpio_out`)


