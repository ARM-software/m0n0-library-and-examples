ADPDev Script
*************

When run, the ``adpdev.py`` script:

1. Connects to the chip via the ADP Interface and instantiates a ``chip`` object (instance of M0N0S2 class, see :ref:`chap-testchip`);
2. Loads embedded software into the Development RAM ("DEVRAM") and sets the chip read instructions from DEVRAM;
3. Optionally loads any trim values (to compensate for chip variation in specific components, such as the RTC oscillator); 
4. Holds the system in reset (unless ``--auto-release`` flag is provided); 
5. Goes into a interactive Python prompt where commands can be issued. 

From the interactive prompt, commands can be issued through the ``chip`` object for debugging/testing. For example, it can be used for:

* Holding/Releasing the CPU reset;
* Changing (or reading) system settings, such as the current DVFS level;
* Run small test utilities, such as measuring the current RTC frequency or deriving an RTC trim value;
* Reading/Writing system registers;
* Sending and Receiving STDIN/STDOUT ("printf");
* Remotely calling embedded software functions and reading pass/fail status. 


Before running the script, ensure the environment and dependencies have been set up (see the `ADPDev README.md <https://github.com/ARM-software/m0n0-library-and-examples/tree/master/adpdev>`_ file).  

For usage instructions, run::

  ./adpdev.py --help


Reference
#########

.. automodule:: adpdev
   :members:
   :undoc-members:
   :show-inheritance:

