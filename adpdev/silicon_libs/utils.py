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
import silicon_libs.testchip as testchip

def bin_search(min_limit, max_limit, depth, func, params, is_int=False):
    """Implement generic binary search algorithm

    :param min_limit: The minimum range limit
    :type min_limit: int or float (must set is_int if integer)
    :param max_limit: The maximum range limit
    :type max_limit: int or float
    :param depth: The recursion depth limit
    :type depth: int
    :param func: Function for evaluating result. Takes in the new value to
                 set and a dictionary of parameters. 
    :type func: function
    :param params: Dictionary of params to pass to to func, including a 
                   logger object
    :param is_int: Set if the values to binary search are integers
    :type is_int: bool, optional
    """
    new_val = (float(max_limit) + float(min_limit))/2.0
    params['logger'].info("-------------------------------")
    if is_int:
        new_val = int(new_val)
        params['logger'].info("min: {}, max: {}, mid: {}".format(
            min_limit,max_limit,new_val))
        params['logger'].info("min: 0x{:04X}, max: 0x{:04X}, mid: {:04}".format(
            min_limit,max_limit,new_val))
    else:
        params['logger'].info("min: {:0.4f}, max: {:0.4f}, mid: {:0.4f}".format(
            min_limit,max_limit,new_val))
    results = func(new_val,params)
    depth -= 1
    if depth < 0:
        params['logger'].info("Maximum depth reached")
        return {'results' : results, 'min' : min_limit, 'max' : max_limit}
    if min_limit == max_limit:
        params['logger'].info("Converged")
        return {'results' : results, 'min' : min_limit, 'max' : max_limit}
    if results['bin_value'] == 1:
        return bin_search(new_val, max_limit, depth, func, params, is_int=is_int)
    elif results['bin_value'] == 0:
        return bin_search(min_limit, new_val, depth, func, params, is_int=is_int)
    else:
        raise ValueError("Invalid bin_value: {}".format(results['bin_value']))

def example_rtc_trim(chip, logger, time_period_s=10, max_iterations=10):
    """Example of a trimming a chips RTC using a linear search. Just an example, use the methods provided in the M0N0S2 class to trim the RTC instead
    """
    # PCSM cannot be read, so using model value (the PoR value unless changed
    # previously by this script). 
    logger.info("Enable FBB (may already be set by embedded software anyway):")
    chip.pcsm.write('rtc_ctrl1',1,bit_group='en_fbb')
    current_rtc_trim = chip.pcsm.read(
            'rtc_ctrl0',bit_group='trim_res_tune')
    logger.info("Starting RTC trim value: 0x{:08X}".format(current_rtc_trim))
    iteration = 0
    while True:
        current_rtc_trim = chip.pcsm.read(
                'rtc_ctrl0',bit_group='trim_res_tune')
        logger.info("Iteration: {:d}:: Trim: 0x{:08X}".format(iteration,
            current_rtc_trim))
        res = utils.measure_rtc(chip, logger, time_period_s) 
        if (res['error_pc'] > 0):
            logger.info("Decreasing trim...")
            chip.pcsm.write(
                    'rtc_ctrl0',
                    current_rtc_trim - 1,
                    bit_group='trim_res_tune')
        else:
            logger.info("Increasing trim...")
            chip.pcsm.write(
                    'rtc_ctrl0',
                    current_rtc_trim + 1,
                    bit_group='trim_res_tune')
 

def derive_adp_port(logger):
    """Automatically derive the ADP port (only works on MacOS (perhaps Linux) and with the Demoboard

    :param logger: The logger object
    :type logger: logger.Logger object
    :return: The full ADP port path
    :rtype: str
    """
    logger.warning("Auto ADP port - Mac (many linux) ONLY!")
    if not os.path.isdir('/dev'):
        raise IOError("Could not open /dev directory. This only works in Mac/Linux")
    adp_ports = [x for x in os.listdir('/dev') if \
             x.startswith('cu.usbserial-A')]
    if len(adp_ports) < 1:
        print(adp_ports)
        raise IOError("Could not find the ADP port")
    if len(adp_ports) > 1:
        raise IOError("Multiple possible ADP ports found: {}".format(
                adp_ports))
    return os.path.join('/dev',adp_ports[0])

def setup_logger(logger, level, log_filepath):
    """Sets up and returns a logger object

    :param logger: The logger object for logging messages to the console
                   and file
    :type logger: logging.Logger object
    :param level: The minimum log level to print to the console
    :type level: str (one of "DEBUG", "INFO", "WARN", "ERROR")
    :param log_filepath: Path to file to log messages to"
    :type log_filepath: str
    :return: The logger object for logging messages to the console
                   and file
    :rtype: logging.Logger object
    """
    temp_logger = logger
    temp_log_level = logging.getLevelName(level)
    logger.setLevel(temp_log_level)
    # create a file handler 
    temp_handler = logging.FileHandler(log_filepath,mode='w')
    temp_handler.setLevel(logging.DEBUG)
    # create a formatter
    temp_formatter = logging.Formatter('%(levelname)s:%(asctime)s:%(name)s:%(filename)s:%(lineno)s:%(funcName)s():%(message)s')
    temp_handler.setFormatter(temp_formatter)
    # add file handler to logger
    temp_logger.addHandler(temp_handler)
    # create handler for screen
    console_handler = logging.StreamHandler()
    console_formatter = logging.Formatter(
            '%(levelname)s (%(filename)s:%(lineno)s):  %(message)s')
    console_handler.setFormatter(console_formatter)
    temp_logger.addHandler(console_handler)
    return temp_logger
 
def process_adp_tx_params(tx_params):
    """Takes the parameter text from ADP TX params section and converts to dictionary

    :param tx_params: The parameter text from the ADP TX parameter section
    :type tx_params: str
    :return: Result dictionary with the keys and values
    :rtype: dict
    """
    # input is a string - convert to dict
    def is_int(value):
        try:
            int(value,0)
            return True
        except ValueError:
            return False
    tx_params = [x.strip() for x in tx_params.strip().split('\n')]
    res = { x.split(':')[0].strip() : int(x.split(':')[1]) if \
             is_int(x.split(':')[1]) else x.split(':')[1].strip() \
             for x in tx_params}
    return res

class AudioReader:
    """Class for decoding audio ADP transactions received from M0N0 (demoboard)
    """
    def __init__(self, logger, save_path='temp.wav'):
        self._logger = logger
        self._save_path = save_path
    
    def demoboard_audio(self, tx_name, tx_params, tx_payload):
        """Decodes the audio data and parameters received via the ADP TX. Saves audio data in a WAVE file.
        
        :param tx_name: The name of the transaction
        :type tx_name: str
        :param tx_params: The raw text from the parameter part of the ADP TX
        :type tx_params: str
        :param tx_payload: The raw text from the payload of the ADP TX
        :type tx_payload: str
        """
        audio_frame = []
        tx_params = process_adp_tx_params(tx_params)
        sample_freq_hz = 8000
        print(tx_params)
        if tx_params:
            if 'sample_freq_hz'in tx_params:
                sample_freq_hz = int(tx_params['sample_freq_hz'])
                self._logger.info("Sample frequency (Hz): {:d}".format(
                        tx_params['sample_freq_hz']))
            if 'period_rtc_ticks'in tx_params:
                self._logger.info("RTC tick period: {:d}".format(
                        tx_params['period_rtc_ticks']))
            record_time_s = None
            if 'recording_rtc_cycles' in tx_params:
                record_time_s = tx_params['recording_rtc_cycles'] * (1.0/33e3)
                self._logger.info("Record time: {:0.2f} s (RTC cycles: {:d})"\
                        .format(
                        record_time_s,
                        tx_params['recording_rtc_cycles']))
        audio_lines = [x.strip() for x in tx_payload.strip().split('\n')]
        audio_words = [int(x,0) for x in audio_lines]
        def twos_comp(val, bits):
            #return val ^ (1<<7)
            return val
            """compute the 2's complement of int value val"""
            if (val & (1 << (bits - 1))) != 0:
                val = val - (1 << bits)
            return val 
        for word in audio_words:
            audio_frame.append(twos_comp((word>>24) & 0xFF, 8))
            audio_frame.append(twos_comp((word>>16) & 0xFF, 8))
            audio_frame.append(twos_comp((word>>8) & 0xFF, 8))
            audio_frame.append(twos_comp((word) & 0xFF,8))
        self._logger.info("Number of samples: {:d}".format(len(audio_frame)))
        if record_time_s:
            sample_freq_hz = int(len(audio_frame) / record_time_s)
            self._logger.info("Calculated freq is: {:d} Hz".format(
                    sample_freq_hz))
        import wave
        import struct
        wavefile = wave.open(self._save_path, 'w')
        wavefile.setparams((1, 1, sample_freq_hz, 0, 'NONE', 'not compressed'))
        for sample in audio_frame:
            sample =  sample + 128
            data = struct.pack('<h', sample)
            wavefile.writeframesraw( data )
        wavefile.close()

"""Returns the default ADP TX callbacks
"""
def get_default_adp_tx_callbacks():
    return {
        'demoboard_audio' : demoboard_audio 
    }
