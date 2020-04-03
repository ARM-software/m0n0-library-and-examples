#!/usr/bin/env python
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
import threading
import os
import sys
import serial
import datetime
import time
from time import clock
import re
import logging

import silicon_libs.read_buffer as read_buffer

Escape = 27
Enter = 10
prompt_timeout = 4  # Long one needed for tgoram tests - do not reduce

class ADPCommsError(Exception):
    pass

class ADP_Conn:
    """Manages an ADP connection

    :param logger: The logger object for logging messages to the console
                   and file
    :type logger: logging.Logger object
    :param sample_period: How often the ADP connection is read into a software buffer (in a separate thread)
    :type sample_period: float
    :param log_directory: path of a directory for logging
    :type log_directory: str, optional
    :param logfile_name: A name that can be added to the logfile
                           filename (i.e. if multiple instances of this
                           class are used)
    :type logfile_name: str, optional
    """
    def __init__(self,
                 logger=False,
                 sample_period=0.05,
                 log_directory = 'logs',
                 logfile_name = None
                 ):
        """Constructor
        """
        self._logger = logger or logging.getLogger(__name__)
        self._tx_log_filename = os.path.join(log_directory, 'adp_transactions'
                +(("-"+logfile_name) if logfile_name else "")+'.log')
        with open(self._tx_log_filename,'w+') as f:
            f.write('')
        f.closed
        self._sample_period = sample_period
        self._port = None
        self._read_buffer = None
        self._prompt_timeout = 5
        self._prompt_pattern = ']'
        self._read_buffer_log_path = os.path.join(log_directory, 'read_buffer'
            +(("-"+logfile_name) if logfile_name else "")+'.log')

    def set_adp_tx_callbacks(self, callbacks):
        """Set callback functions to call when ADP transaction of specific names are received via ADP
        """
        self._read_buffer.set_adp_tx_callbacks(callbacks)

    def close(self):
        """Closes the ADP port
        """
        self._logger.info("Closing ADP Port")
        if self._port:
            self._port.close()

    def _log_transaction(self, message, is_write=False):
        text = '\n>>>|WRITE|' if is_write else '<<<|READ|'
        text += str(datetime.datetime.now())
        text += '|'
        with open(self._tx_log_filename,'a') as f:
            f.write(text+str(message))
        f.closed
        with open(self._tx_log_filename+'-stripped.log','a') as f:
            f.write(text+str(message).replace('>', '').replace('<', ''))
        f.closed

    def open(self, port_path, timeout=1, print_received=False):
        """Opens an ADP port connection

        :param port_path: Path to the serial port to use
        :type port_path: str
        :param timeout: The amount of time (in seconds) to wait in all serial
                        communications before an error is thrown if nothing
                        is received
        :type timeout: float
        """
        # Display start message
        self._logger.info("Opening ADP port: " + port_path)
        # Connect to ADP
        self._port = serial.Serial(port=port_path, timeout=timeout)
        # Report on success
        if (self._port is None):
            self._logger.error("Could not open port: " + port)
            raise IOError("Could not open port {}".format(port))
        else:
            self._logger.debug("ADP port opened successfully")
        # Clear buffer
        self._port.flush()
        self._port.reset_input_buffer()
        self._port.reset_output_buffer()
        self._logger.debug("Setting up read buffer")
        self._read_buffer = read_buffer.ReadBuffer(
            self._port,
            self._sample_period,
            self._read_buffer_log_path,
            logger=self._logger,
            tx_log_func = self._log_transaction,
            print_received=print_received
        )
        self._read_buffer.create_buffer('adp')
        time.sleep(1)

        # Get prompt
        self._get_prompt()

    def _get_prompt(self):
        # Get ADP prompt
        result = 1
        for i in range(0,2):
            if i > 0:
                self._logger.info("Re-sending prompt request...")
            self._logger.debug('get prompt')
            self._read_buffer.flush_buffer('adp')
            self._log_transaction("%c%c%c%c" % (Escape, Enter, Enter, Enter),is_write=True)
            temp = self._port.write(b"%c%c%c%c" % (Escape, Enter, Enter, Enter))
            tries = 1
            result = self._check_prompt(100)
            while result and tries:
                self._logger.error("Timeout in checking prompt. Retrying...")
                result = self._check_prompt(100)
                tries -= 1
            if not result:
                break
        if result:
            raise ADPCommsError("Cannot Communicate with ADP")
        return result


    def _check_prompt(self, ID):
        # Start time
        self._logger.debug('check prompt '+str(ID))
        new_approach = True
        if new_approach:
            stop_time = time.time() + self._prompt_timeout
            while time.time() < stop_time:
                prompt_loc = self._read_buffer.read_only_buffer(
                    'adp').find(self._prompt_pattern)
                if prompt_loc > -1:
                    # probably don't need this as we now always clear 'adp'
                    self._read_buffer.read_buffer(
                        'adp', prompt_loc+len(self._prompt_pattern))
                    return 0
            self._logger.error("Timed out: No prompt found: Error ID" +
                              str(ID) + " - A restart might be required")
            return 1
        else:
            startTime = clock()
            data = ""
            # Read characters until a prompt ("]") is found
            # If the operation times out, the method will end
            while data != "]":
                if self._read_buffer.in_waiting('adp') > 0:
                    self._logger.debug("startTime: {}, clock-startTime: {} prompt_timeout: {}".format(
                        startTime,
                        startTime-clock(),
                        prompt_timeout))
                    data = self._read_buffer.read_buffer('adp', 1)
                elif clock() - startTime >= prompt_timeout:
                    time.sleep(0.1)

    def memory_write(self, address, data):
        """ Write data to any address location in the memory map
        
        :param address: The memory address
        :type address: int
        :param data: The value to write
        :type data: int
        """
        self._logger.debug('memory write.. A:{:X}, Data:{:X}'.format(address, data))
        if type(address) == str:
            raise ValueError("String addresses no longer supported")
        self.write("A 0x{:08X}\n".format(address))
        self._check_prompt(101)
        # Write data
        if type(data) == str:
            raise ValueError("String data no longer supported")
        self.write("W 0x{:08X}\n".format(data))
        self._check_prompt(102)
        return 1

    def write_bin(self, data):
        # Write data over serial
        self._port.write(data)

    def read_bin(self):
        self._port.read()

    def write(self, data):
        # Write data over serial
        self._log_transaction(data,is_write=True)
        self._port.write(data.encode('utf-8'))
        #self._logger.debug('Write: {}'.format(data.strip()))

    def memory_read(self, address):
        """ Read any address location in the memory map
        
        :param address: The memory address
        :type address: int
        :return: the value at that address
        :rtype: int
        """
        # Write address
        # Change the hex to string without the L suffix
        self._read_buffer.flush_buffer('adp')
        self._logger.debug('memory read.. {}'.format(address))
        if type(address) == str:
            raise ValueError("String addresses no longer supported")
        #self.write("A %s\n" % address)
        self.write("A 0x{:08X}\n".format(address))
        self._check_prompt(103)

        self._read_buffer.flush_buffer('adp')
        # Read address
        self.write("R\n")
        ##Read in data
        result = self.poll_buffer_for_pattern(
                'adp',
                r"0x[\dA-F]{8}",
                period=0.01,
                timeout=1,
                remove_angle_brackets=True)
        self._logger.debug("poll buffer result: {}".format(result))
        if result['timed_out']:
            raise ADPCommsError("Read timed out")
        data = result['match_only']
        self.write("\n")
        self._check_prompt(103)
        # Return data
        return(int(data,0))
   
    def memory_dump_bin(self, bin_path, address, limit):
        """ Write a binary image to a memory region (fast)
        
        :param bin_address: Path to binary file to load
        :type bin_path: str
        :param address: The start address
        :type address: int
        :param limit: The maximum length to write (bytes)
        :type limit: int
        """
        self._logger.debug("Binary memory dump")
        time.sleep(1)
        if type(address) == str:
            raise ValueError("String addresses no longer supported")
        # Open file and check size
        written_chars = [] 
        with open(bin_path, mode='rb') as f:
            bytecount = os.path.getsize(bin_path)
            if bytecount > limit:
                self._logger.warn("Bytecount is over limit")
                bytecount = limit
            # Initiate binary transfer
            chunk_size = 32768
            if bytecount > chunk_size:
                self._logger.info("Writing in multiple chunks")
            for chunk in range(0, (int(bytecount/chunk_size))+1):
                lower_i = chunk * chunk_size
                upper_i = lower_i + (chunk_size if (bytecount - lower_i) > chunk_size else bytecount - lower_i)
                lower_addr = lower_i + address
                upper_addr = upper_i + address
                data_size = upper_addr - lower_addr
                self._logger.info("Chunk: {:d}, size: 0x{:08X} ({:d}), 0x{:08X} - 0x{:08X} ({:d} - {:d})".format(
                    chunk,
                    data_size,
                    data_size,
                    lower_addr,
                    upper_addr-1,
                    lower_addr,
                    upper_addr-1
                ))
                self._logger.debug("orig: A 0x{:08X}".format(address))
                self._logger.debug("orig: B 0x{:08X}".format(bytecount))
                self._logger.debug("new: A 0x{:08X}".format(lower_addr))
                self._logger.debug("new: B 0x{:08X}".format(data_size))
                self.write("A 0x{:08X}\n".format(lower_addr))
                self.write("B 0x{:08X}\n".format(data_size)) # format required is B+space+bytes
                self._log_transaction("*",is_write=True) # lets not do this for every single byte!
                write_count = 0
                for wr in range(lower_i, upper_i):
                    ch=f.read(1)
                    written_chars.append(ch)
                    if not ch:
                        raise ValueError("No character")
                        break
                    if not (write_count % 1024):
                        self._logger.debug("Write sleep...")
                        time.sleep(0.1)
                    self.write_bin(ch);
                    write_count += 1
                # Write new line (to avoid adp lockup if code doesn't include one)
                self.write('\n')
                time.sleep(0.2)
        f.close()
        # Flush buffer
        self.flush_output()
        self.memory_read(address) # important: waits for prompts to go away
   
    def memory_dump(self, hex_path, address, limit):
        """ Write a hex image to a memory region (slow) - old implementation

        :param hex_path: Path to the hex file to load
        :param address: The start address
        :param limit: The maximum length to write (words)
        """
        if not isinstance(address, int):
            raise ValueError("Only integer address now supported")
        self._logger.debug("memory dump")
        prefix = 1
        time.sleep(1)
        StackPointer = None
        MainPointer = None
        # Write address
        self.write("A 0x{:08X}\n".format(address))
        # Number of lines written
        count = 0
        # Open file
        Hex_FileObj = open(hex_path, mode='r')
        # For each line
        for line in Hex_FileObj:
            # if count == 0:
            ##    StackPointer = line
            # if count == 1:
            ##    MainPointer = line
            # if line == '\n':
            # break

            # Write data
            self.write("W "+line)

            # Display data
            #self._logger.debug("W prefix: {}, line: {}".format(W_Prefix, line.strip()))
            #self._logger.debug("Count: {}".format(count))
            # Windows computers hang 2182 so lets flush every 2000 writes
            if(count % 2000 == 0):
                self.flush_output()
            # If number of lines exceeds limit
            count = count + 1
            if(count > limit):
                self._logger.info("Count Reached = " + count)
                break
        # Write new line (just in case dhrystone code doesn't include one)
        # Without the new line, new commands can't be sent
        self.write('\n')

        # Close file
        Hex_FileObj.close()

        # Flush buffer
        self.flush_output()
        self.flush_output()
        self.memory_read(address) # important: waits for prompts to go away


    def memory_read_batch_fast(self, address, words):
        """Fast batch memory read function
        
        :param address: The start/base address (word-aligned)
        :type address: int
        :param words: The number of words to read
        :type words: int
        :return: List of integer word values at each word address
        :rtype: list
        """
        self._logger.info("Started (fast) memory read batch. Address: 0x{:08X}, Words: {:d}".format(
                address,
                words))
        if type(address) == str:
            raise ValueError("String addresses no longer supported")
        self._read_buffer.create_buffer('batch')
        self.flush_output()
        self._read_buffer.flush_buffer('batch')
        time.sleep(1)
        self.write("\nA 0x{:08X}\n".format(address))
        received = ""
        vals = []
        debug_count = 0
        # Appears to only work 16 words at a time
        last_len = 0
        chunk_size = words if words < 64 else 64
        chunk = 0
        for w in range(0, words):
            #print('word: {}'.format(w))
            self.write("\nR\n")
            debug_count += 1
            if not (w+1) % chunk_size:
                #print("----"+str(chunk_size)+"----")
                self._logger.info("Chunk {:d} (size: {:d}). Total Words: 0x{:08X}, Bytes: {:d}".format(
                        chunk,
                        chunk_size,
                        w+1,
                        (w+1)*4))
                last_len = self._read_buffer.in_waiting('batch')
                while True:
                    time.sleep(0.1)
                    # wait for received data to stop
                    new_len = self._read_buffer.in_waiting('batch')
                    if new_len == last_len:
                        # stopped receiving so break
                        temp = self._read_buffer.read_all_buffer('batch')
                        all_matches = re.findall(
                                r"^]*(0x[0-9A-F]{8})", temp, re.MULTILINE)
                        #print("this length: {:d}".format(len(temp)))
                        if len(all_matches) != chunk_size:
                            self._logger.info("Expected length: {}, actual length: {}".format(chunk_size, len(all_matches)))
                            raise ValueError("Memory read data length does not match")
                        vals.extend([int(x,0) for x in all_matches])
                        received += temp
                        #print("Finished Length: {:d}".format(len(received)))
                        time.sleep(0.05)
                        self.write("\nA 0x{:08X}\n".format(address+(chunk*chunk_size)))
                        self.flush_output()
                        self._read_buffer.flush_buffer('batch')
                        chunk += 1
                        break
                    last_len = new_len
            time.sleep(0.015)
        self._logger.info("Expected length: {}, actual length: {}".format(words, len(vals)))
        if words != len(vals):
            self._logger.error("Memory read data length does not match")
        return vals

    """ For batch reads of memory (implementation from original ADP scripts but with read buffer
    """
    def memory_read_batch(self, address, words):
        self._logger.info("Started memory read batch")
        if not isinstance(address, int):
            raise ValueError("Only integer address now supported")
        time.sleep(0.5) # wait for prompts to be received
        self._read_buffer.create_buffer('batch')
        self.flush_output()
        self._read_buffer.flush_buffer('batch')
        time.sleep(1)
        self.write("\nA 0x{:08X}\n".format(address))
        c = ''
        while(c != ']'):
            if self._read_buffer.in_waiting('batch') > 0:
                c = self._read_buffer.read_buffer('batch', 1)
        self.write("R\n")
        c = ''
        while(c != ']'):
            if self._read_buffer.in_waiting('batch') > 0:
                c = self._read_buffer.read_buffer('batch', 1)
        i = 0
        word = ''
        data = ''
        while(i < words):
            if self._read_buffer.in_waiting('batch') > 0:
                # print("Something")
                tempchar = self._read_buffer.read_buffer('batch', 1)
                #raw_input("%s:, %x" % (tempchar, ord(tempchar)))
                word += tempchar
                if len(word) == 10:
                    data += word + ' '
                    self.write("\n")
                    c = ''
                    while(c != ']'):
                        if self._read_buffer.in_waiting('batch') > 0:
                            c = self._read_buffer.read_buffer('batch', 1)
                    self.write("R\n")
                    word = ''
                    i += 1
                    self._logger.debug(
                        "Completed word {:010d}/{:010d}".format(i, words))
            else:
                pass
                # print("Nothing")
        #time.sleep(1) # wait for prompts to be received
        self.memory_read(address) # dummy read, reads incorrectly
        self._logger.info("Finished batch read")
        return data

    def _adp_enter(self):

        self.write("%c" % (Escape))
        self.write("\n")
        self.write("\n")

    def _adp_exit(self):
        self.write("\n")
        self.write("X")
        self.write("\n")

    def stdin(self, string):
        """Send text to device via ADP STDIN (standard input)

        :param string: Text to write via standard input
        :type string: str
        """
        self._adp_exit()
        for s in string:
            self.write(s)
        self._adp_enter()

    def flush_output(self):
        """Clears all data in output buffer
        """
        self._port.reset_output_buffer()

    def create_buffer(self, buffer_name):
        """Creates an independent software buffer in which any received data (including ADP command response and printf/stdout) is inserted into. 

        :param buffer_name: Name of the buffer to create
        :type buffer_name: str
        """
        self._logger.debug("Creating buffer: {}...".format(buffer_name))
        self._read_buffer.create_buffer(buffer_name)

    def read_all_buffer(self, buffer_name):
        """Reads and clears the selected buffer (destructive read) independently of other defined buffers

        :param buffer_name: The name of the existing buffer
        :type buffer_name: str
        :return: All data in the buffer
        :rtype: str
        """
        self._logger.info("Reading all buffer: {}...".format(buffer_name))
        return self._read_buffer.read_all_buffer(buffer_name)

    def flush_buffer(self, buffer_name):
        self._logger.info("Flusing buffer: {}...".format(buffer_name))
        self._read_buffer.flush_buffer(buffer_name)

    def read_only_buffer(self, buffer_name):
        """Reads the selected buffer (without clearing) independently of other defined buffers

        :param buffer_name: The name of the existing buffer
        :type buffer_name: str
        :return: All data in the buffer
        :rtype: str
        """
        self._logger.info("Read only buffer: {}...".format(buffer_name))
        self._read_buffer.read_only_buffer(buffer_name)

    def poll_buffer_for_string(self,
                               buffer_name,
                               string,
                               period=0.5,
                               timeout=1,
                               remove_angle_brackets=False):
        """Monitors a buffer until a specific string is detected or a timeout occurs

        :param buffer_name: name of the existing buffer to poll
        :type buffer_name: str
        :param string: The string to match in the buffer
        :type string: str
        :param period: The time interval (in seconds) at which to test
                       for a match
        :type period: float
        :param timeout: The time (in seconds) after which to abort if no match
                        has been found
        :type timeout: float
        :param remove_angle_brackets: Printf output has angled brackets around
                                      each character. Enabling this option
                                      removes all angled brackets from the text
                                      (whether it is in printf or not) before
                                      testing the match
        :type remove_angle_brackets: bool, optional
        """
        stop_time = time.time() + timeout
        while time.time() < stop_time:
            text = self._read_buffer.read_only_buffer(buffer_name)
            if remove_angle_brackets:
                text = text.replace('>', '').replace('<', '')
            if text.find(string) > -1:
                return {'printf': text, 'timed_out': False}
            time.sleep(period)
        return {'printf': '', 'timed_out': True}

    def poll_buffer_for_pattern(self,
                               buffer_name,
                               expr,
                               period=0.05,
                               timeout=1,
                               remove_angle_brackets=False):
        """Monitors a buffer until a specific regular expression (regex) is matched or a timeout occurs

        :param buffer_name: name of the existing buffer to poll
        :type buffer_name: str
        :param expr: The regular expression (regex) to match in the buffer
        :type string: str
        :param period: The time interval (in seconds) at which to test
                       for a match
        :type period: float
        :param timeout: The time (in seconds) after which to abort if no match
                        has been found
        :type timeout: float
        :param remove_angle_brackets: Printf output has angled brackets around
                                      each character. Enabling this option
                                      removes all angled brackets from the text
                                      (whether it is in printf or not) before 
                                      testing the match
        :type remove_angle_brackets: bool, optional
        """
        stop_time = time.time() + timeout
        text = ''
        while time.time() < stop_time:
            text = self._read_buffer.read_only_buffer(buffer_name)
            if remove_angle_brackets:
                text = text.replace('>', '').replace('<', '')
            #if text.find(string) > -1:
                #return {'printf': text, 'timed_out': False}
            match_obj = re.search(expr, text, re.MULTILINE) 
            if match_obj is not None: 
                return {'whole_text': text, 'timed_out': False, 'match_only' : match_obj.group(0)}
            time.sleep(period)
        return {'whole_text': text, 'timed_out': True, 'match_only' : ''}
