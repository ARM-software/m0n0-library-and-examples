#!/usr/bin/env python2
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
import serial
import datetime
import time
import collections
import atexit
import re

TRAILING_PRINTF_REGEX = r"<([^>]*?)$"
#TRAILING_PRINTF_REGEX = r"<([^>]*?)$"
LEADING_PRINTF_REGEX = r"^([^<]*?)>"
INNER_PRINTF_REGEX = r"<([\s\S].*?)>"

LOOK_FOR_TX = True
ADP_TX_KEY = "3d7db2aetx"
ADP_TC_MATCH_REGEX = r"3d7db2ae_tx_start<<(.*?)>>\n([\s\S]*?)3d7db2ae_tx_end<<(.*)>>"
ADP_TC_MATCH_REGEX = r"3d7db2ae_tx_start<<(.*?)>>\n(([\s\S]*?)(3d7db2ae_params_end))?([\s\S]*?)3d7db2ae_tx_end<<(.*)>>"

class ReadBuffer:
    """Samples the serial port in a separate thread and puts the read data into the defined software buffers (that can independently created, flushed and deleted.
    """
    def __init__(
            self,
            ser_port,
            sample_period,
            log_file,
            buffer_length=65536,
            tx_log_func=None,
            print_received=False,
            logger=None):
        self._logger = logger or logging.getLogger(__name__)
        self._ser_port = ser_port
        self._sample_period = sample_period
        self._log_file = log_file
        self._log_file_stripped = log_file+'-stripped.log'
        self._log_file_printf = log_file+'-stdout.log'
        self._print_received = print_received
        self._buffer_length = buffer_length
        self._tx_log_func = tx_log_func
        self._read_thread = threading.Thread(
                target=self._periodic_sample, args=[])
        self._buffers = {}
        self._printf_buffers = {}
        self._stop = False
        self._adp_tx_callbacks = {}
        with open(self._log_file, 'w') as f:
            f.write('____LOG_FILE_CREATED: ' +
                    datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
        f.closed
        with open(self._log_file_stripped, 'w') as f:
            f.write('____LOG_FILE_CREATED: ' +
                    datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
        f.closed
        with open(self._log_file_printf, 'w') as f:
            f.write('')
        f.closed
        if LOOK_FOR_TX:
            self.create_printf_buffer("adp_tx_lookout");
        atexit.register(self._stop_thread)
        self._read_thread.setDaemon(True)
        self._read_thread.start()

    def set_adp_tx_callbacks(self, callbacks):
        self._adp_tx_callbacks = callbacks

    def _stop_thread(self):
        self._stop = True

    def create_buffer(self, buffer_name):
        """Creates a software buffer that it will push the received data to
        """
        self._buffers[buffer_name] = collections.deque(
            maxlen=self._buffer_length)

    def create_printf_buffer(self, buffer_name):
        """ Creates a software buffer that it will push M0N0 printf to only
        """
        self._printf_buffers[buffer_name] = collections.deque(
            maxlen=self._buffer_length)

    def read_buffer(self, buffer_name, num_chars=1):
        """ Destructively reads (pops) from buffer. 
        """
        if num_chars > len(self._buffers[buffer_name]):
            num_chars = len(self._buffers[buffer_name])
        return ''.join([self._buffers[buffer_name].popleft()
                for x in range(0, num_chars)])

    def read_printf_buffer(self, buffer_name, num_chars=1):
        """ Destructively reads (pops) from printf buffer.
        """
        if num_chars > len(self._printf_buffers[buffer_name]):
            num_chars = len(self._printf_buffers[buffer_name])
        return ''.join([self._printf_buffers[buffer_name].popleft()
                for x in range(0, num_chars)])

    def flush_buffer(self, buffer_name):
        """ Clears the buffer
        """
        self._buffers[buffer_name].clear()

    def flush_printf_buffer(self, buffer_name):
        """ Clears the printf buffer
        """
        self._printf_buffers[buffer_name].clear()

    def read_only_buffer(self, buffer_name):
        """ Non-destrive read of entire buffer
        """
        return ''.join(list(self._buffers[buffer_name]))

    def read_only_printf_buffer(self, buffer_name):
        """ Non-destrive read of entire printf buffer
        """
        return ''.join(list(self._printf_buffers[buffer_name]))

    def read_all_buffer(self, buffer_name):
        """Destructive read of entire buffer
        """
        # reads until empty
        result = ''.join(list(self._buffers[buffer_name]))
        self._buffers[buffer_name].clear()
        return result

    def read_all_printf_buffer(self, buffer_name):
        """ Destructive read of entire printf buffer
        """
        # reads until empty
        result = ''.join(list(self._printf_buffers[buffer_name]))
        self._printf_buffers[buffer_name].clear()
        return result

    def in_waiting(self, buffer_name):
        """Gets the number of characters unread from buffer
        """
        return len(self._buffers[buffer_name])

    def in_printf_waiting(self, buffer_name):
        """Gets the number of characters unread from printf buffer
        """
        return len(self._printf_buffers[buffer_name])

    def _periodic_sample(self):
        """Samples the input read buffer periodically
        """
        count = 0
        carryover_printf = "" # for storing printf spanning across samples
        while self._stop == False:
            count += 1
            data = ''
            if self._ser_port.isOpen() == False:
                break
            try:
                while self._ser_port.inWaiting() > 0:
                    data += ((self._ser_port.read(1)).decode('utf-8') )
                    if self._ser_port.isOpen() == False:
                        break
            except OSError:
                self._logger.warn("Read Buffer Terminated")
                break
            if data:
                temp_printf = ''
                leading_printf_match = re.search(LEADING_PRINTF_REGEX, data)
                trailing_printf_match = re.search(TRAILING_PRINTF_REGEX, data)
                inner_printf_match = re.findall(INNER_PRINTF_REGEX, data)
                if leading_printf_match:
                    temp_printf = carryover_printf + leading_printf_match.group(1)
                carryover_printf = ''
                if trailing_printf_match:
                    carryover_printf = trailing_printf_match.group(1)
                if inner_printf_match:
                    for m in inner_printf_match:
                        if isinstance(m, str):
                            temp_printf += m
                        else:
                            temp_printf += m(1)
                for key, value in self._buffers.items():
                    self._buffers[key].extend(data)
                for key, value in self._printf_buffers.items():
                    self._printf_buffers[key].extend(temp_printf)
                with open(self._log_file_printf, "a") as f:
                    f.write(temp_printf)
                f.closed
                with open(self._log_file, "a") as f:
                    f.write(data)
                f.closed
                with open(self._log_file_stripped, "a") as f:
                    f.write(data.replace('>', '').replace('<', ''))
                f.closed
                if self._tx_log_func:
                    self._tx_log_func(data,is_write=False)
                if self._print_received:
                    print(data)
                if LOOK_FOR_TX:
                    # run regex
                    res = re.search(
                        ADP_TC_MATCH_REGEX,
                        self.read_only_printf_buffer("adp_tx_lookout"),
                        re.MULTILINE)
                    if (res):
                        self.flush_printf_buffer('adp_tx_lookout')
                        tx_type = res.group(1)
                        if tx_type != res.group(6):
                            raise ValueError("ADP TX start and end types do not match")
                        tx_params = res.group(3)
                        tx_content = res.group(5)
                        self._logger.info("Received ADP TX ({}, chars: {}, lines: {})".format(
                            tx_type,
                            len(tx_content),
                            len(tx_content.split('\n'))
                        ))
                        self._logger.info("Params: {}".format(tx_params))
                        print("self._adp_tx_callbacks")
                        print(self._adp_tx_callbacks)
                        if tx_type in self._adp_tx_callbacks:
                            self._adp_tx_callbacks[tx_type](tx_type, tx_params, tx_content)
            time.sleep(self._sample_period)
        self._logger.info("Read Buffer thread terminated")

    def print_buffers(self):
        print("Printing...")
        for key, value in self._buffers.iteritems():
            print("Buffer "+str(key)+": "+self._buffers[key])
