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
import time

import silicon_libs.testchip as testchip
import silicon_libs.utils as utils

# Paths
LOG_FILEPATH = os.path.join('logs', 'adpdev.log')
TRIM_PATH = "silicon_libs/trims/M0N0S2/trims.yaml"


def main(params, logger):
    """The main function. Sets up a chip and enters an interactive prompt

    :param params: Command line arguments and chip options
    :type params: dict
    :param logger: The logger object for logging messages to the console
                   and file
    :type logger: logging.Logger object
    """
    params['load_devram'] = True  # always load DEVRAM
    params['load_trims'] = TRIM_PATH if params['trim'] else None
    logger.info("Started ADPDEV...")
    chip = testchip.M0N0S2.setup_chip(
            params,
            logger,
            skip_reload=params['skip_reload'])
    # Pass special callbacks for interpreting M0N0 STDOUT
    # for general ADPDev testing, these are not required:
    audio_reader = utils.AudioReader(logger)
    chip.set_adp_tx_callbacks({
        'demoboard_audio': audio_reader.demoboard_audio
    })
    # Custom code can go here
    # Go to an interactive python prompt:
    code.interact(local=dict(globals(), **locals()))
    if chip:
        chip.exit()

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(
            description="Script for connecting to the M0N0 chip, loading "
                        "software and debug")
    parser.add_argument(
            '--chip-id',
            required=True,
            help="The Chip ID (written on the top of the chip directly under "
                 "'M0N0-S2')")
    parser.add_argument(
            '--adp-port',
            required=True,
            help="Specify the ADP port address")
    parser.add_argument(
            '-l',
            '--log-level',
            required=False,
            choices=['DEBUG', 'INFO', 'WARNING', 'ERROR'],
            default="INFO",
            help="Sets the logger level for console output")
    parser.add_argument(
            '-s',
            '--software',
            required=True,
            help="Software directory containing the bin/hex and/or testcase"
                 " list")
    parser.add_argument(
            '--trim',
            required=False,
            action='store_true',
            default=False,
            help="If specified, the trims for this chip (based on Chip ID) "
                 "will be loaded. The chip must have been previously  "
                 "measured and have the default values in the trims file.")
    parser.add_argument(
            '--auto-release',
            required=False,
            action='store_true',
            default=False,
            help="If specified, the reset is released automatically after"
                 " loading DEVRAM")
    parser.add_argument(
            '--show-ports',
            required=False,
            action='store_true',
            default=False,
            help="Utility for showing all available serial ports")
    parser.add_argument(
            '--skip-reload',
            required=False,
            action='store_true',
            default=False,
            help="Skips the loading of DEVRAM (for quickly testing python "
                 "script changes)")
    args = parser.parse_args()
    if args.show_ports:
        os.system('python -m serial.tools.list_ports')
        exit()
    if not os.path.exists('logs'):
        os.makedirs('logs')
    # Setup logger for filtered log message to console and file
    temp_logger = utils.setup_logger(
            logging.getLogger(__name__),
            args.log_level,
            LOG_FILEPATH)
    temp_logger.info("Set up logger")
    # Parse the command-line arguments (as a dict) and logger to main:
    main(vars(args), temp_logger)
