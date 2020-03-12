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

import serial
import logging
import time
import re

class MbedSocket:  # For individual register - not yet used
    def __init__(self,port,logger=None,baud_rate=9600,timeout=0.5):
        self._logger = logger or logging.getLogger(__name__)
        self._logger.info("Connecting to MBED on port: {}".format(port))
        self._mbed_sock = serial.Serial(
                port=port,
                #parity=serial.PARITY_NONE,
                #stopbits=serial.STOPBITS_ONE,
                baudrate=9600,
                #bytesize=serial.EIGHTBITS,
                #xonxoff=False,
                #dsrdtr=True,
                #rtscts=True,
                timeout=timeout);
        if not self._mbed_sock:
            self._logger.error("Could not connect to MBED")
            raise IOError("Could not open mbed socket!")
        self._mbed_sock.flush();
        self._mbed_sock.reset_input_buffer();
        self._mbed_sock.reset_output_buffer();

    def close(self):
        self._logger.info("Closing MBED port")
        if self._mbed_sock:
            self._mbed_sock.close()

    def tx(self,data):
        self._mbed_sock.flush()
        self._mbed_sock.reset_input_buffer()
        self._mbed_sock.reset_output_buffer()
        self._logger.debug('Sending "{}" to MBED'.format(data.strip()))
        #to_send = '\nrdadc 4\n'
        for d in data:
            self._mbed_sock.write(d.encode('utf-8'))
            time.sleep(0.1)
        received = self._mbed_sock.read(1000) # reads 100 chars or 0
        decoded = received.decode('utf-8')
        self._logger.debug("MBED Received: {}".format(decoded))
        res = re.search(
            r'=(.*)$',
            decoded,
            re.MULTILINE)
        if not res:
            return None
        res = res.group(1) 
        self._logger.debug("MBED Result: {}".format(res))
        return res

    def tx_raw(self,data):
        self._mbed_sock.flush()
        self._mbed_sock.reset_input_buffer()
        self._mbed_sock.reset_output_buffer()
        self._logger.debug('Sending "{}" to MBED'.format(data.strip()))
        for d in data:
            self._mbed_sock.write(d.encode('utf-8'))
            time.sleep(0.1)
        received = self._mbed_sock.read(1000) # reads 100 chars or 0
        decoded = received.decode('utf-8')
        self._logger.debug("MBED Received: {}".format(decoded))
        return decoded

    def read_temperature(self):
        #self.mbed.tx('setfreq b 132\n')
        return float(self.tx('rdadc 4\n'))

    def read_vreg(self):
        return float(self.tx('rdadc 3\n'))

    def read_vdev(self):
        return float(self.tx('rdadc 2\n'))

    def read_vbat(self):
        return float(self.tx('rdadc 1\n'))

    def por(self):
        self._logger.info("Sending POR")
        self.tx('porpulse\n') 

    def external_wake(self):
        self._logger.info("Sending EXTWAKE")
        self.tx('ewpulse 1000\n') 

    def read_and_log(self,num):
        read_data = self._mbed_sock.read(num)
        print("READ: {}".format(read_data))
        print("DECODED: {}".format(read_data.decode()))
        return read_data

    def tx_old(self,data):
        data_enc = bytes(data, encoding="ascii")
        ret=''
        for i in range(0,len(data_enc)):
            self._mbed_sock.write(data_enc[i])
            time.sleep(0.1)
        i+=1
        while(i and self._mbed_sock.inWaiting()):
            ret += self.read_and_log(1)
            i-=1
        print('MbedTx: cmd: {} retChar: {}'.format(data_enc, ret))
        ret = ''
        time.sleep(0.5)
        while(self._mbed_sock.inWaiting()):
            ret += self.read_and_log(1)
        print(ret)
        return ret


