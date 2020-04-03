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
# the ADPDev libraries. 

# Define parameter dictionary to be passed to chip setup function
chip_params = {
    'chip_id' : 0, # This should be set to the chip ID written on the package
    'adp_port' : "/dev/cu.usbmodem2072379E584E1", # Serial COM port for ADP
    'software' : "../projects/hello_world/build/", # dir. with bin to load
    'load_devram' : True, # whether to load DEVRAM (should be True)
    'load_trims' : "silicon_libs/trims/M0N0S2/trims.yaml", # path to trims DB
    'auto_release' : False, # whether to automatically release the reset
}

# create a logger - for logging messages to console and log file
logger = utils.setup_logger(
        logging.getLogger(__name__),
        "INFO", # Log filter options: "DEBUG", "INFO", "WARN", "ERROR"
        "logs/barebone_example.log")
logger.info("Completed logger setup")

# create chip object
chip = testchip.M0N0S2.setup_chip(
        chip_params,
        logger)
logger.info("Completed chip setup")
# chip is now set up and ready to run by releasing the reset

# a good way to test/debug is by switching to an interactive console:
# code.interact(local=dict(globals(), **locals()))


# The remainder of this script is an example
logger.info("Running example")
chip.reset_release()


# To stop the ADP monitoring and close the connection:
chip.exit()

