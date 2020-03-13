# DVFS Example

This training example shows how to update change the DVFS level. 

A working example is provided in the `devhat_example_dvfs` project. 

Goals:

1. Setup the extake button interrupt in `main`
2. Print the current perf and estimated TCRO frequency if extwake is pressed
3. Implement the `setup_systick` function to call the systick callback function (which toggles the LEDs) every 2000000 TCRO cycles
4. Set the new perf after printing `sys->log_info("Old DVFS: %d, new DVFS: %d", current_dvfs, new_dvfs);`


