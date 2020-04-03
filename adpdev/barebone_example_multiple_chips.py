#!/usr/bin/env python3
################################################################################
# Copyright (c) 2020, Arm Limited
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
################################################################################

import os
import logging
import code  # for going into interactive session

import silicon_libs.testchip as testchip
import silicon_libs.utils as utils

# This script shows the bare minimum for setting up a custom script using
# the ADPDev libraries - but for connecting to multiple chips
# simultaneously

# Define parameter dictionary to be passed to the chip setup function
chip_a_params = {
    'chip_id' : 0, # This should be set to the chip ID written on the package
    'adp_port' : None, # Serial COM port for ADP
    'software' : "../projects/hello_world/build/", # dir. with bin to load
    'load_devram' : True, # whether to load DEVRAM (should be True)
    'load_trims' : "silicon_libs/trims/M0N0S2/trims.yaml", # path to trims DB
    'auto_release' : False, # whether to automatically release the reset
    'chip_name' : 'chip_a' # explicitly specifies text to add to log file names
}

chip_b_params = {
    'chip_id' : 1, # This should be set to the chip ID written on the package
    'adp_port' : None, # Serial COM port for ADP
    'software' : "../projects/hello_world/build/", # dir. with bin to load
    'load_devram' : True, # whether to load DEVRAM (should be True)
    'load_trims' : "silicon_libs/trims/M0N0S2/trims.yaml", # path to trims DB
    'auto_release' : False, # whether to automatically release the reset
    'chip_name' : 'chip_b' # explicitly specifies text to add to log file names
}

# create a logger - for logging messages to console and log file
logger = utils.setup_logger(
        logging.getLogger(__name__),
        "INFO", # Log filter options: "DEBUG", "INFO", "WARN", "ERROR"
        "logs/barebone_example.log")
# The same logger can be passed to multiple testchips, however, it is 
# convenient to distinguish which messages are from which chip instances
chip_a_logger = utils.setup_logger(
        logging.getLogger("chip_a"),
        "INFO", # Log filter options: "DEBUG", "INFO", "WARN", "ERROR"
        "logs/barebone_example.log",
         prefix="CHIP A")
chip_b_logger = utils.setup_logger(
        logging.getLogger("chip_b"),
        "INFO", # Log filter options: "DEBUG", "INFO", "WARN", "ERROR"
        "logs/barebone_example.log",
         prefix="CHIP B")
logger.info("Completed setting up multiple loggers")

# create chip object
logger.info("Setting up Chip A...")
chip_a = testchip.M0N0S2.setup_chip(
        chip_a_params,
        chip_a_logger)
logger.info("Setting up Chip B...")
chip_b = testchip.M0N0S2.setup_chip(
        chip_b_params,
        chip_b_logger)
logger.info("Completed chip setup")
# chip is now set up and ready to run by releasing the reset

# a good way to test/debug is by switching to an interactive console:
# code.interact(local=dict(globals(), **locals()))


# The remainder of this script is an example
logger.info("Running example")


# To stop the ADP monitoring and close the connection:
chip_a.exit()
chip_b.exit()

