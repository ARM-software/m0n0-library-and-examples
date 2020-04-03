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
    """Stores database of testcases in the embedded software and enables them to be commanded to run, and results collected. 

    :param path_to_tc_table: Path the testcase-list.csv file
    :type path_to_tc_table: str
    :param adp_sock: The ADP Socket
    :type adp_sock: adp_socket.ADP_Conn
    :param ctrl_reg: The M0N0 System control register object
    :type ctrl_reg: registers_model.Registers_Model
    :param logger: The logger object for logging messages to the console
                   and file
    :type logger: logging.Logger object
    :param tcs_to_run_path: A path to a list of testcase names to run, one-after another
    :type tcs_to_run_path: str
    """
    def __init__(self,
            path_to_tc_table,
            adp_sock,
            ctrl_reg,
            logger=False,
            tcs_to_run_path = None
            ):
        """ Constructor
        """
        self._logger = logger or logging.getLogger(__name__)
        self._logger.info("Initialising Testcase_Controller")
        self._adp_sock = adp_sock
        self._db = _create_testcase_db(path_to_tc_table,logger=self._logger)
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
        """Returns the list of testcases to run, one-after-another (if set)
        
        :return: The testcases list to be run
        :rtype: list containing str elements
        """
        return self._tcs_to_run

    def get_testcase_list(self):
        """Returns the names of the testcases in the testcase database (e.g. loaded from the testcase-list.csv)

        :return List of testcases in the embedded software that can be run
        """
        return self._db['data']['Testcase_Enum']

    def get_testcase_id(self,name):
        """Gets the ID of a testcase from its name. This is what is sent to the chip to command it to run the testcase. Derived from the position of the testcase in the testcase list. 

        :param name: Name of the testcase (e.g. "HELLO", "EN_D_SLEEP") to find the ID of
        :type name: str
        :return: The ID
        :rtype: int
        """
        return self._db['data']['Testcase_Enum'].index(name)

    def __str__(self):
        res = ""
        res += "\n".join(self._db['data']['Testcase_Enum'])
        return res

    def __getitem__(self, index):
        return self._db['data']['Testcase_Enum'][index]

    def set_wakeup_testcase(self,tc,repeat_delay=None):
        """ Sets a testcase to be executed after wakeup (requires specific embedded software support and is for internal testing only)
        """
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
        """Commands a specific testcase in the embedded software in the chip to run a testcase (the embedded software must be waiting in a specific "wait_for_adp" loop to receive this command)

        :param tc: The name of the testcase to run, or the ID of the testcase to run
        :type tc: str, int
        :param wait_for_output: If specified, it waits for the testcase to complete (and collects result) before continuing. If True is passed, then it waits until the standard STDOUT indicating the end of a testcase is received. If a string is passed then it waits until it matches that string in the STDOUT. If an integer is passed, then it waits for that number of seconds. 
        :type wait_for_output: bool, str, int, optional
        :param regex_funcs: A list of functions that apply regex expressions to the printf output to derive key results from the testcase. The default options passes a list with a single function to match the standard testcase pass/fail flags. These functions must return a dictionary and the keys,values of this dictionary are added to the results dictionary that is returned. 
        :type regex_funcs: list of functions, optional
        :param measure_funcs: A list of functions to call if repeating the workload for measurements (see `repeat_delay`). These callback functions are called, one-by-one, in order. These functions must return a dictionary and the keys,values of this dictionary are added to the results dictionary that is returned. 
        :type measure_funcs: list of functions, optional
        :param repeat_delay: If None, False or 0, the workload is run once (for normal pass/fail testing). If it is greater than 0, then the workload is repeated (and printf suppressed) for the number specified (in seconds) and the `measure_funcs` run (if specified). 
        :type repeat_delay: bool, int, optional
        :param timeout: If waiting to the testcase to finish, then how long to wait before giving up and reporting a timeout (in seconds). Note that when using `repeat_delay` then this value is modified in the code to scale. See the code for details. 
        :type timeout: bool, int, float, optional
        :return: If waiting for the output, then a dictionary containing generic results (such as settings [e.g. workload name], start and end time), specific results extracted from the printf output from any `regex_funcs`, and any measurement results from the `measure_funcs`. 
        :rtype: None, dict
        """
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



def _create_testcase_db(path, logger=False):
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


    
