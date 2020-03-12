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

import time
import datetime
import silicon_libs.regex_lib as regex_lib

wait_adp_str = '<W><a><i><t><i><n><g>< ><f><o><r>< ><A><D><P>< ><d><i><r><e><c><t><i><o><n>'

class TestcaseController:
    def __init__(self,
            path_to_tc_table,
            adp_sock,
            ctrl_reg,
            logger=False,
            tcs_to_run_path = None
            ):
        self._logger = logger or logging.getLogger(__name__)
        self._logger.info("Initialising Testcase_Controller")
        self._adp_sock = adp_sock
        self._db = create_testcase_db(path_to_tc_table,logger=self._logger)
        self._ctrl_reg = ctrl_reg
        self._tcs_to_run = []
        if tcs_to_run_path:
            self._set_tcs_to_run(tcs_to_run_path)

    def _set_tcs_to_run(self,path_to_tcs_to_run_file):
        with open(path_to_tcs_to_run_file,'r') as f:
            self._tcs_to_run = [x.strip() for x in 
                    f.read().split('\n') if len(x) > 0 and not '#' in x]
        f.closed
        self._logger.info("TCs to run: {}".format(self._tcs_to_run))
        invalid_tcs = []
        for t in self._tcs_to_run:
            if t not in self.get_testcase_list():
                invalid_tcs.append(t) 
        if len(invalid_tcs) > 0:
            raise ValueError("Some TCs to run not in TC List: {}".format(
                    invalid_tcs))

    @property
    def tcs_to_run(self):
        return self._tcs_to_run

    def get_testcase_list(self):
        return self._db['data']['Testcase_Enum']

    def get_testcase_id(self,name):
        return self._db['data']['Testcase_Enum'].index(name)

    def __str__(self):
        res = ""
        res += "\n".join(self._db['data']['Testcase_Enum'])
        return res

    def __getitem__(self, index):
        return self._db['data']['Testcase_Enum'][index]

    # Uses TGO 
    def set_wakeup_testcase(self,tc,repeat_delay=None):
        if not isinstance(tc,str):
            raise ValueError("Testcase name must be a string")
        if not repeat_delay:
            repeat_delay = 0
        repeat_delay_raw = int((repeat_delay*132000) / 4096)
        tgo_base = chip.mem_map.get_base('SHRAM') 
        logger.info("TGO base: 0x{:X}".format(tgo_base))
        temp_tgo = adp_sock.memory_read(tgo_base)
        temp_cafe = adp_sock.memory_read(tgo_base+0x4)
        logger.info('Original TGO[0]...0x{:X}'.format(temp_tgo))
        tc_index = tc.get_testcase_id(tc)
        # set to run testcase (tgoid) after reset:
        adp_sock.memory_write(tgo_base,0xC0FFEE00|tc_index)
        #adp_sock.memory_write(S1.TGOSRAM_BASE+0x4,0xCAFE0000|11) # repeat 11*4096 rtc cyc.
        adp_sock.memory_write(tgo_base+0x4,0xFEED0000|repeat_delay_raw) 
        temp_tgo = adp_sock.memory_read(tgo_base)
        temp_cafe = adp_sock.memory_read(tgo_base+0x4)
        logger.info('Reading TGO[0]...{:X}'.format(temp_tgo))
        logger.info('Reading TGO[1]...{:X}'.format(temp_cafe))
        return {
            "wakeup_tc_name" : tc,
            "wakeup_tc_index" : tc_index,
            "wakeup_tc_repeat_delay" : repeat_delay,
            "wakeup_tc_repeat_delay_raw" : repeat_delay_raw
        }

    def check_wakeup_testcase(self,expected_tc,timeout=3):
        raise NotImplementedError("Not yet implemented this")

    def run_testcase(
                self,
                tc,
                wait_for_output=None,
                regex_funcs=[regex_lib.testcase_result_decode],
                measure_funcs=None,
                repeat_delay=None,
                timeout=1):
        tc_table = self._db
        adp = self._adp_sock
        ctrl_reg = self._ctrl_reg
        tc_index = -1
        if isinstance(tc, str):  # note, in Python3 this is isinstance(s, str)
            # string, so look up in table
            if not tc_table:
                raise ValueError("No tc_table!")
            tc_index = tc_table['data']['Testcase_Enum'].index(tc)
            self._logger.info("About to run TC: {}: {} ({})".format(
                tc_index, tc, tc_table['data']['Testcase_Function'][tc_index]))
        else:
            # integer, don't bother with table
            self._logger.warning("TC Specified with Index - use label")
            tc_index = tc
            if (tc_table):
                self._logger.info("About to run TC: {}: {} ({})".format(
                    tc_index, tc, tc_table['data']['Testcase_Function'][tc_index]))
        if tc_index < 0:
            raise ValueError("Invalid TC index!")
        # now actually run the TC!
        if tc_index > 255:
            raise ValueError(
                'tc_index ({}) is larger than 255!'.format(tc_index))
        result = {}
        result['TC_ID'] = tc_index
        result['TC_name'] = tc_table['data']['Testcase_Enum'][tc_index]
        result['TC_function'] = tc_table['data']['Testcase_Function'][tc_index]
        if not repeat_delay:
            repeat_delay = 0
        repeat_delay = int((repeat_delay*33000) / 4096)
        timeout += repeat_delay*1.2
        adp.create_buffer('temp_tc_buffer')
        ctrl_reg.write('CTRL_5',0xFFFFFFFE)  # clear all but strobe
        ctrl_reg.write('CTRL_5', (0x00000000 | ((tc_index << 8)) | (repeat_delay << 16))) # clear all but strobe
        ctrl_reg.write_clear('CTRL_5',0x00000001)
        ctrl_reg.write_set('CTRL_5',0x00000001)
        result['start_time'] = datetime.datetime.now()
        start_time = time.time()
        printf_out = 'not set'
        printf_strip = 'not set'
        if measure_funcs:
            for func in measure_funcs:
                result.update(func())
        measure_finish_time = time.time()
        if wait_for_output:
            if wait_for_output == True:
                result['howrun'] = 'wait_for_ADP_string'
                self._logger.info("Waiting for program output by matching specific string")
                poll_result = (adp.poll_buffer_for_string(
                    'temp_tc_buffer', wait_adp_str,timeout=timeout))
                timed_out = poll_result['timed_out']
                printf_out = poll_result['printf']
                self._logger.debug("Received program output")
                self._logger.debug("Program output: {}".format(printf_out))
            elif isinstance(wait_for_output, str):
                result['howrun'] = 'wait_for_string'
                self._logger.info("Waiting for program output by matching string")
                poll_result = (adp.poll_buffer_for_string(
                    'temp_tc_buffer', wait_for_output, timeout=timeout))
                timed_out = poll_result['timed_out']
                printf_out = poll_result['printf']
                self._logger.debug("Received program output")
                self._logger.debug("Program output: {}".format(printf_out))
            elif isinstance(wait_for_output, (int)) or isinstance(wait_for_output, numbers.Number):
                result['howrun'] = 'wait_for_time'
                self._logger.info("Waiting for program output by waiting for a time ({})".format(
                        wait_for_output))
                time.sleep(wait_for_output)
                printf_out = adp.read_all_buffer('temp_tc_buffer')
                self._logger.debug("Received program output")
                self._logger.debug("Program out: {}".format(printf_out))
            else:
                raise ValueError(
                    "Wait for output must be string or number if true")
            end_time = time.time()
            result['measure_finish_time'] = measure_finish_time
            measure_time_margin = end_time - measure_finish_time
            self._logger.debug("MEASURE TIME MARGIN: {}".format(measure_time_margin))
            if measure_time_margin < 2 and repeat_delay:
                raise ValueError("Measure time margin too small")
            result['end_time'] = datetime.datetime.now()
            result['duration'] = end_time - start_time
            if printf_out:
                result['printf'] = printf_out
                result['printf_strip'] = printf_out.replace(
                    '>', '').replace('<', '')
            else:
                result['printf'] = ''
                result['printf_strip'] = ''
            # run devram regex
            # this might want to be a function passed in for flexibility!
            for o in regex_funcs:
                self._logger.debug("Executing regex func: {}".format(o))
                func_name = o.__name__
                self._logger.debug("Function name: {}".format(func_name))
                temp_res = o(result['printf_strip'], self._logger)
                for k in temp_res:
                    result[func_name+'-'+k] = temp_res[k]
            return result
        else:
            return



def create_testcase_db(path, logger=False):
    # purposely not using pandas
    logger = logger or logging.getLogger(__name__)
    import re
    lines = []
    with open(path, 'r') as f:
        lines = f.read().split('\n')
    f.closed
    headers = []
    db = {
        'header': [],
        'data': {}
    }
    for i in range(0, len(lines)):
        line = lines[i]
        if i == 0:
            db['header'] = line.split()
            for h in db['header']:
                db['data'][h] = []
        else:
            line_vals = line.split()
            for v in range(0, len(line_vals)):
                db['data'][db['header'][v]].append(line_vals[v])
    n = len(db['data'][db['header'][0]])
    # check all same length
    assert all(len(db['data'][h]) == n for h in db['header'])
    logger.debug(db['header'])
    for h in db['header']:
        logger.debug("\n"+h+":  ")
        logger.debug(db['data'][h])
    return db


    
