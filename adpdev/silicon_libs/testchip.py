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

from abc import ABC, abstractmethod
import sys
import os
import logging
import yaml
import time
import collections  # for ordered dict
sys.path.append("registers_models/")
import registers_model as registers_model
import mem_map as mem_map_lib
import silicon_libs.adp_socket as adp_socket
import silicon_libs.mbed_socket as mbed_socket
import silicon_libs.testcase_control as tc_lib
import silicon_libs.utils as utils


class SpiTimeoutError(Exception):
    pass


class DummyDrivers:
    """Dummy read/write drivers (for testing code without a real interface)

    :param logger: The logger to use for logging messages to console and file
    :type logger: logging.Logger object, optional
    """
    def __init__(self, logger=None):
        """Constructor method
        """
        self._logger = logger or logging.getLogger(__name__)

    def dummy_read(self, address):
        """The "dummy" read function which does not read and just logs a warning.

        :param address: The address to do a dummy read from
        :type address: int
        :return: Always returns 0
        :rtype: int
        """
        self._logger.warn("DUMMY DEVICE READ: A: 0x{:08X}".format(address))
        return 0

    def dummy_write(self, address, data):
        """The "dummy" write function which does not write and just logs a warning

        :param address: The address to do a dummy write to
        :type address: int
        :param data: The data to dummy write
        :type address: int
        :return: Always returns True
        :rtype: bool
        """
        self._logger.warning("DUMMY DEVICE WRITE: A 0x{:08X} D: 0x{:08X}".format(
                address,
                data))
        return True


class TestChip(ABC):
    """An abstract base class representing a chip under test. Specific chips to be derived from this base class. 
    
    :param logger: The logger to use for logging messages to console and file
    :type logger: logging.Logger object, optional
    :param mbed_port: Serial port address for companion test microcontroller
    :type mbed_port: str, optional
    :param chip_id: The ID of the chip under test
    :type chip_id: int
    :param sw_directory: The path to a directory containing software to run or
                         information relating to software already in the chip
    :type sw_directory: str
    :param load_devram: A flag indicating whether to load the Development RAM
                        (or equivalent memory)
    :type load_devram: bool
    :param tcs_to_run_path: A path to a file with a list of testcases 
                            (workloads inside software) to run one-after
                            -another.
    :type tcs_to_run_path: str
    :param load_trims: A path to a file containing a YAML trim database with
                       trim values to load for the specific chip (identified
                       with the chip ID). If not specified, then no trims are
                       loaded. 
    :type load_trims: str, optional
    :param chip_name: A name for the chip that is added to log filenames
                      and other logged information to identify which chip
                      the logged information belongs to (i.e. intended for
                      use when multiple chips are connected simultaneously).
                      E.g. Could be a string-representation of the chip ID or
                      a nickname. 
    :type chip_name: str, optional
    """

    _testchip_count = 0

    def __init__(self,
            logger=None,
            mbed_port=None,
            chip_id=None,
            sw_directory=None,
            load_devram=False,
            tcs_to_run_path=None,
            load_trims=None,
            chip_name=None
        ):
        TestChip._testchip_count += 1
        self._logger = logger or logging.getLogger(__name__)
        self._logger.info("Setting up TestChip")
        self._dummy_drivers = DummyDrivers(logger=self._logger)
        self._chip_id = chip_id
        self._tc_ctrl = None
        self._adp_port = None
        self._adp_sock = None
        self._ctrl_regs = None
        self._status_regs = None
        self._gpio_regs = None
        self._spi_regs = None
        self._pcsm_regs = None
        self._tcs = None
        self._sw_directory = sw_directory
        self._tcs_to_run_path = tcs_to_run_path
        self._load_devram = load_devram
        self._chip_name = chip_name
        if self._testchip_count > 1:
            self._chip_name = self._chip_name if self._chip_name \
                     else "{:03d}".format(self.chip_id)
        if chip_id is not None:
            self._logger.info("Chip ID: {:03d}".format(self._chip_id))
        else:
            self._logger.warn("No Chip ID")
        self._logger.info("Chip name: {}".format(self._chip_name))
        self._mem_map = None 
        # MBED Connection
        self._mbed_port = mbed_port or None
        self.mbed = None
        if self._mbed_port:
            self._logger.debug("Setting up MBED")
            self.mbed = mbed_socket.MbedSocket(mbed_port, logger=self._logger)
        else:
            self._logger.warn("No MBED Port. Not using MBED in Testchip") 
        self._initialise()

    @property
    def adp_sock(self):
        """Returns the ADP Socket object for reading and writing directly via the ADP connection
        
        :return: The ADP Socket object
        :rtype: adp_socket.ADP_Conn object
        """
        return self._adp_sock

    @property
    def ctrl_regs(self):
        """Returns the object for reading and writing to/from the M0N0 Control Registers
        
        :return: The Control Register object
        :rtype: registers_model.Registers_Model object
        """
        return self._ctrl_regs

    @property
    def status_regs(self):
        """Returns the object for reading and writing to/from the M0N0 Status Registers
        
        :return: The Status Register object
        :rtype: registers_model.Registers_Model object
        """
        return self._status_regs

    @property
    def gpio_regs(self):
        """Returns the object for reading and writing to/from the M0N0 GPIO Registers
        
        :return: The GPIO Register object
        :rtype: registers_model.Registers_Model object
        """
        return self._gpio_regs

    @property
    def spi_regs(self):
        """Returns the object for reading and writing to/from the M0N0 SPI Registers
        
        :return: The SPI Register object
        :rtype: registers_model.Registers_Model object
        """
        return self._spi_regs

    @property
    def pcsm_regs(self):
        """Returns the object for reading and writing to/from the M0N0 PCSM Registers
        
        :return: The PCSM Register object
        :rtype: registers_model.Registers_Model object
        """
        return self._pcsm_regs

    @property
    def pcsm(self):
        """ Alias for pcsm_regs property
        """
        return self._pcsm_regs



    @property
    def mem_map(self):
        """Returns the object for reading information about the memory map (e.g. base address or size)
        
        :return: The Memory Map object
        :rtype: registers_model.MemoryMap object
        """
        return self._mem_map

    @property
    def tcs(self):
        """Returns the object for working with the database of testcases in the ROM/DEVRAM/CODERAM software
        
        :return: The Testcases object
        :rtype: testcase_control.TestcaseController object
        """
        return self._tcs

    @property
    def perf_labels(self):
        """Returns the perf labels list (e.g. min, high, max etc.)
        
        :return: A list of the perf lables strings
        :rtype: list
        """
        return self._perf_labels



    def load_trim_settings(self, load_trims):
        """Loads the trim for the current chip using the specified YAML trim database and chip ID. 
        
        :param load_trims: Path to the trim database YAML file
        :type load_trims: str
        """
        self._logger.info("Loading trims from: {}".format(load_trims))
        trim_db = {}
        with open(load_trims, 'r') as f:
            trim_db = yaml.safe_load(f)
        f.close()
        trim_db = trim_db['chip']
        if self._chip_id in trim_db:
            for key in trim_db[self._chip_id]:
                if key != 'pcsm':
                    self._logger.error("Currently only supports pcsm trim")
                else:
                    for k,v in trim_db[self._chip_id][key].items():
                        if isinstance(v, int):
                            self._logger.info("Sending trim: self._pcsm_regs.write"
                                              "({},{})".format(k,v))
                            self._pcsm_regs.write(k,v)
                        else:
                            for k2,v2 in v.items():
                                if not isinstance(v2,int):
                                    raise("Expected int: {} {} {}".format(
                                            v,v2,k2))
                                else:
                                    self._logger.info("Sending trim: self._pcsm_regs"
                                                      ".write({},{},bit_group"
                                                      "={})".format(
                                                            k,v2,k2))
                                    self._pcsm_regs.write(k,v2,bit_group=k2)
        else:
            self._logger.error("No trim values for this chip"
                               " ({:03d})".format(self._chip_id))
    def connect(self,
            adp_port,
            skip_reload=False,
            load_trims=False,
            safe_tcro=False
        ):
        """Connects to the chip via ADP
        
        :param adp_port: Path to ADP serial port
        :type adp_port: str
        :param skip_reload: Flag specifying whether to skip any loading of software (i.e. in repetitive testing where VDEV power is not interrupted between tests)
        :type skip_reload: bool, optional
        :param load_trims: Flag indicating whether to load trim values
        :type load_trims: bool, optional
        :param safe_tcro: Internal testing feature (do not use)
        :type safe_tcro: bool, optional
        """
        self._logger.info("Connecting to Device")
        # ADP Connection
        self._adp_port = adp_port
        self._logger.info("Setting up ADP")
        self._adp_sock = adp_socket.ADP_Conn(
                logger=self._logger,
                logfile_name=self._chip_name
                ) 
        self._logger.debug("Opening ADP Socket")
        self._adp_sock.open(adp_port, print_received=False) 
        # add ADP drivers (overriding DUMMY drivers where appropriate)
        self._ctrl_regs.set_read_driver(self._adp_sock.memory_read)
        self._ctrl_regs.set_write_driver(self._adp_sock.memory_write)
        self._status_regs.set_read_driver(self._adp_sock.memory_read)
        self._status_regs.set_write_driver(self._adp_sock.memory_write)
        self._gpio_regs.set_read_driver(self._adp_sock.memory_read)
        self._gpio_regs.set_write_driver(self._adp_sock.memory_write)
        self._spi_regs.set_read_driver(self._adp_sock.memory_read)
        self._spi_regs.set_write_driver(self._adp_sock.memory_write)
        self._pcsm_regs.set_write_driver(self._adp_to_pcsm_write)
        if safe_tcro:  # unused
            temp_perf = self.get_perf()
            self._pcsm_regs.write('tcro_ctrl', 0xFC)
            if (temp_perf == 31):
                self.set_perf(25)
            else:
                self.set_perf(31)
            time.sleep(0.2)
            self.set_perf(temp_perf)
        # load trims
        if load_trims:
            self.load_trim_settings(load_trims)
        # initialise software
        self.initialise_software(skip_reload=skip_reload)

    def initialise_software(self, skip_reload=False):
        """Sets up the chip software - loads software (if necessary) and loads the testcases (tcs - workloads built into binary) database (if necessary)
        
        :param skip_reload: Flag specifying whether to skip the reloading of 
                            software (i.e. for repetitive testing without VDEV
                            interruptions)
        :type skip_reload: bool, optional
        """
        # Set up DEVRAM and testcases
        if self._sw_directory:
            # Get bin path (if there) and load it if (load_devram=True) 
            # gets testcase list and optionally loads devram
            self._logger.info("Setting up software...")
            res = self.setup_software(
                    self._sw_directory,
                    self._load_devram,
                    skip_reload=skip_reload)
            if res['testcase_list']:
                self._tcs = tc_lib.TestcaseController(
                        res['testcase_list'],
                        self._adp_sock,
                        self._ctrl_regs,
                        logger=self._logger,
                        tcs_to_run_path=self._tcs_to_run_path)
            else:
                self._tcs = None 
                self._logger.warn("No testcase db, cannot run testcases")
        else:
            self._logger.warn("No sw directory, cannot load or use tcs")

    @abstractmethod
    def _initialise(self):
        pass

    def exit(self): 
        """ Cleanly closes the chip (closing ports and read threads)
        """
        if self._adp_sock:
            self._adp_sock.close()
        if self.mbed:
            self.mbed.close()
        self._logger.info("Chip Exited")

    def setup_software(self, sw_dir, load_devram=False, skip_reload=False):
        """Analyses the provided software directory and works out what files to load (if necessary) and sets up testcase (tc) database (if necessary)

        :param sw_dir: Path to software directory to load (or already loaded 
                       in chip)
        :type sw_dir: str 
        :param load_devram: Whether to load the DEVRAM or assume it persists
        :type load_devram: bool, optional
        :param skip_reload: Whether to skip reloading of memory
        :type skip_reload: bool, optional
        """
        # find testcase list and bin/hex32
        self._logger.info("Looking in {} for .bin/.hex32 and testcase_list.csv"
                          " files".format(sw_dir))
        result = {'testcase_list' : None, 'code_path' : None}
        testcase_list = [x for x in os.listdir(sw_dir) if x == \
                'testcase_list.csv']
        binfile = [x for x in os.listdir(sw_dir) if x.endswith('.bin')]
        hex32 = [x for x in os.listdir(sw_dir) if x.endswith('.hex32')]
        if len(binfile) > 1:
            raise IOError("Multiple bin files!")
        if len(hex32) > 1:
            raise IOError("Multiple hex32 files!")
        if len(testcase_list) > 1:
            raise IOError("Multiple testcase list files!")
        if len(binfile) > 0:
            result['code_path'] = os.path.join(sw_dir, binfile[0]) 
        else:
            if len(hex32) > 0:
                result['code_path'] = os.path.join(sw_dir, hex32[0])
        if load_devram:
            if result['code_path'] == None :
                raise IOError("No hex32 or bin file to load!")
            self._logger.info("flashing: {} to DEVRAM".format(
                    result['code_path']))
            self.reset_hold()
            # Change mem map for booting from DEVRAM
            self.remap_to_devram()
            if skip_reload:
                self._logger.warn("Skipping DEVRAM load")
            else:
                self.flash(result['code_path'], mem_map_location='DEVRAM')
        if len(testcase_list) > 0:
            result['testcase_list'] = os.path.join(sw_dir, testcase_list[0])
        return result

    def remap_to_rom(self):
        """Remaps the memory map so that instructions are executed from ROM
        """
        self._logger.info("Remapping to ROM")
        self._pcsm_regs.write('code_ctrl', 0, bit_group='memory_remap')

    def remap_to_devram(self):
        """Remaps the memory map so that instructions are executed from DEVRAM
        """
        self._logger.info("Remapping to DEVRAM")
        self._pcsm_regs.write('code_ctrl', 1, bit_group='memory_remap')

    def remap_to_coderam(self):
        """Remaps the memory map so that instructions are executed from CODERAM
        """
        self._logger.info("Remapping to CODERAM")
        self._pcsm_regs.write('code_ctrl',1,bit_group='memory_remap')

    def read_memory_region(self, name, words=None, save_to_file=None):
        """ Read content a memory map region

        :param name: The name of the memory region (as per the memory map
                     in the register models)
        :type name: str
        :param words: The number of words to read. Reads entire region if 
                      "None"
        :type words: int, optional
        :param save_to_file: The path of a file which to write the content 
                             to. Does not write file if "None"
        :type save_to_file: str, optional
        :return: Returns a list of data words (as ints)
        :rtype: list
        """
        base = self._mem_map.get_base(name)
        size = self._mem_map.get_size(name)
        start = time.time()
        number_of_words = (words if words else int(size/4))
        mem_words = self._adp_sock.memory_read_batch_fast(base, number_of_words)
        self._logger.info("Read 0x{:08X} words ({:d} bytes) in "
                          "{:0.2f} seconds".format(
                                  number_of_words,
                                  number_of_words*4,
                                  time.time() - start))
        if save_to_file:
            with open(save_to_file, 'w') as f:
                f.write("\n".join(["0x{:08X}".format(x) for x in mem_words]))
            f.close()
        return mem_words

    def flash(self, code_path, mem_map_location='DEVRAM', check_lines=10):
        """Writes binary/hex file to a memory region
        
        :param code_path: Path to binary or hex file to load
        :type code_path: str
        :param mem_map_location: The memory region to write the memory
                                 to (as per the name in the memory map table
                                 defined in the registers_models
        :type mem_map_location: str
        :param check_lines: The number of words ("lines") to check have been
                            written correctly, starting from the beginning. 
        """
        # Hold reset
        self.reset_hold()
        # Write devram code
        temp_base = self._mem_map.get_base('DEVRAM')
        temp_size = self._mem_map.get_size('DEVRAM')
        # measure and report time to upload, binary can be up to 6x faster
        start = time.time()
        if code_path.endswith('.bin') :
            self._logger.info("Writing binary file")
            self._adp_sock.memory_dump_bin(code_path, temp_base, temp_size)
        else:
            self._logger.info("Writing hex file")
            self._adp_sock.memory_dump(code_path, temp_base, temp_size)
        end = time.time()
        self._logger.info("Write time: {:0.2f} seconds".format(end-start))
        if code_path.endswith('.hex32') :
            if check_lines:
                hex_lines = []
                with open(code_path, 'r') as f:
                    hex_lines = [x.strip() for x in f.readlines()]
                f.closed
                if isinstance(check_lines, (int)):
                    hex_lines = hex_lines[0:check_lines]
                    self._logger.info(
                            "Print first {:d} HEX and memory lines for"
                            " sanity".format(
                                    check_lines))
                devram_lines = []
                start = time.time()
                devram_lines = [x.lower() for x in 
                        self._adp_sock.memory_read_batch(
                                temp_base, check_lines).split()]
                self._logger.info("Read 0x{:08X} words ({:d} bytes) in "
                                  "{:0.2f} seconds".format(
                                        check_lines,
                                        check_lines*4,
                                        time.time() - start))
                self._logger.debug(devram_lines)
                self._logger.debug("Length of memory lines: {}".format(
                        len(devram_lines)))
                self._logger.debug("Length of hex lines: {}".format(
                        len(hex_lines)))
                match_fails = []
                for l in range(0, len(hex_lines)):
                    h_line = hex_lines[l]
                    dev_line = devram_lines[l]
                    match_fail = True
                    if "0x{}".format(h_line) == "{}".format(dev_line):
                        match_fail = False
                    self._logger.info("CHECK: File: 0x{}  memory: {}."
                                      " is match?={}".format(
                                            h_line,
                                            dev_line,
                                            not match_fail))
                    match_fails.append(match_fail)
            if any(match_fails):
                raise ValueError("Memory does not match HEX!")
    
    def reset(self,delays=None):
        """Resets the chip
        """
        self.reset_hold()
        time.sleep(0.1)
        self.reset_release()
        
    @abstractmethod
    def reset_hold(self):
        """Holds the reset
        """
        pass

    @abstractmethod
    def reset_release(self):
        """Releases the reset
        """
        pass

    @staticmethod
    @abstractmethod
    def setup_chip(params, logger, chip=None, skip_reload=False):
        """ A non-member utility function for setting up the chip with command-line options

        :param params: Parameters, including forwarded command line options
        :type params: dict
        :param logger: The logger to use for logging messages to console and file
        :type logger: logging.Logger object, optional
        :param chip: An old chip instance can be passed to safely close and
                     restart with a new object
        :type chip: testchip.TestChip or derived
        :param skip_reload: Bypasses the hex/binary write (usually to DEVRAM)
                            - assumes VDEV has not been reset and saves 
                            experiment time
        :type skip_reload: bool, optional
        :return: The new chip object
        """
        pass


class M0N0S2(TestChip):

    _perf_label_lookup =  {
        'min' : 28,
        'low' : 29,
        'mid_low' : 30,
        'mid' : 26,
        'mid_high' : 27,
        'high' : 23,
        'max' : 19
    }

    _perf_labels = ['min','low','mid_low','mid','mid_high','high','max']

    RTC_PERIOD_US = 30.3030303  
    RTC_ONE_MS_TICKS = 33;

    @property
    def perf_labels(self):
        """Returns the perf labels list (e.g. min, high, max etc.)
        
        :return: A list of the perf lables strings
        :rtype: list
        """
        return self._perf_labels

    @property
    def ordered_perfs(self):
        """Returns a list of perfs (HW IDs) in order (lowest TCRO frequency to highests)

        :return: List of integers of the perf (DVFS) HW ID
        :rtype: list
        """
        return [ 28,24,29,20,30,25,31,16,26,21,27,22,17,23,18,19 ]

    @property
    def vbat_max(self):
        """Maximum safe VBAT voltage value without causing chip damage (used to set instrument safety limits)
        
        :return: Voltage level
        :rtype: float
        """
        return 1.6

    @property
    def chip_id(self):
        """Returns the Chip ID
        
        :return: Chip ID
        :rtype: int
        """
        return self._chip_id

    def perf_to_freq_estimate(self, perf):
        """Returns an estimation of the TCRO frequency for the specified perf level using previously characterised example values

        :param perf: The perf level to get a TCRO estimation for. Can be a perf label (str) or a HW ID (int)
        :type perf: int or str
        :return: perf
        :rtype: float
        """
        if isinstance(perf, (str)):
            perf = self._perf_label_lookup[perf]
        return self._pcsm_regs.get_value_table('perf_ctrl','perf')[perf]

    def set_perf(self, perf):
        """Sets the current perf (DVFS level) of the chip

        :param perf: The perf level to change to as a HW ID (int) or perf 
                     label (str)
        :type perf: int or str
        """
        self._logger.info("Setting perf to: {}".format(perf))
        if isinstance(perf, (int)):
            self._pcsm_regs.write(
                    'perf_ctrl',
                    perf,
                    bit_group='perf')     
        elif isinstance(perf, (str)):
            self._pcsm_regs.write(
                    'perf_ctrl',
                    self._perf_label_lookup[perf],
                    bit_group='perf')     
        else:
            raise ValueError("Perf must be int or string")

    def get_perf(self):
        """Gets the current perf (DVFS level) of the chip (read from STATUS7)

        :return: The current perf (HW ID)
        :rtype: int
        """
        return self._status_regs.read('STATUS_7',bit_group="perf")

    def set_dvfs(self, level):
        """Sets the current perf (DVFS level) using the perf ID (0-15)
        
        :param level: Perf level (0-15)
        :type level: int
        """
        if level < 0 or level > 15:
            raise ValueError("DVFS Leve must be 0-15")
        perf = self.ordered_perfs[level]
        self._logger.info("Setting DVFS level to {} (HW ID: {})".format(
                level, perf))
        self._pcsm_regs.write(
                'perf_ctrl',
                perf,
                bit_group='perf')     

    def get_dvfs(self):
        """Reads the current perf level (0-15) using STATUS7
        
        :return: The current perf level (0-15)
        :rtype: int
        """
        return self.ordered_perfs.index(
                self._status_regs.read('STATUS_7',bit_group="perf"))

    def get_rtc(self, unit=None):
        """Reads the current RTC counter value
        
        :param unit: The unit of the return value. Options: "cycles" 
                     (raw RTC clock cycles), "us" (microseconds), "ms"
                     (milliseconds), "s" (seconds). Default is RTC cycles
        :type unit: str, optional
        :return: The RTC ticks (int) or time in the specified units (float)
        :rtype: int or float
        """
        lsbs = self._status_regs.read('STATUS_2')
        msbs = self._status_regs.read('STATUS_4',bit_group="rtc_msbs")
        val = lsbs | (msbs << 32)
        if not unit:
            return val
        unit_mults = {
            'cycles': 1,
            'us' : self.RTC_PERIOD_US,
            'ms' : self.RTC_PERIOD_US/(1000.0),
            's' : self.RTC_PERIOD_US/(1000.0*1000.0)
        }
        if unit not in unit_mults.keys():
            raise ValueError("get_rtc unit parameter must be one of"
                    + " {}".format(possible_units))
        return val*unit_mults[unit]
 
    def get_cfsr(self):
        """Returns the Configurable Fault Status Register (CFSR) value
        
        :rtype: int
        """
        res = self._adp_sock.memory_read(0xE000ED28)
        return res

    def power_off_roms():
        """Powers off the ROM banks (which switch on automatically as required)
        """
        self._ctrl_regs.write("CTRL_2", 0)

    def get_results(self):
        """Returns an ordered dict of key current system variables for logging to a results file
        
        :rtype: collections.OrderedDict
        """
        d = collections.OrderedDict()
        d['chip_model'] = 'M0N0S2'
        d['chip_id'] = self._chip_id
        d['adp_port'] = self._adp_port
        d['mbed_port'] = self._adp_port
        d['sw_directory'] = self._sw_directory
        d['in_reset'] = True if self._ctrl_regs.read('CTRL_0',
                            bit_group='master_reset') else False
        remap =  self._status_regs.read('STATUS_7', bit_group='memory_remap')
        if remap == 0:
            d['remap_to'] = "ROM"
        elif remap == 1:
            d['remap_to'] = "DEVRAM"
        elif remap == 2:
            d['remap_to'] = "CODERAM"
        else:
            raise ValueError("Invalid Memory Remap")
        d['deve_core'] = self._status_regs.read(
                'STATUS_7', bit_group='deve_core')
        d['perf_modelled'] = self._pcsm_regs.read_model(
                'perf_ctrl', bit_group='perf')['val']
        d['perf'] = self._status_regs.read('STATUS_7',bit_group='perf')
        perf_label = 'NA'
        if int(d['perf']) in self._perf_label_lookup.values():
            for name, level in self._perf_label_lookup.items():
                if level == d['perf']:
                    perf_label = name
                    break
        d['ctrl_2_dump'] = self._ctrl_regs.read('CTRL_2')
        d['ctrl_4_dump'] = self._ctrl_regs.read('CTRL_4')
        d['status_3_dump'] = self._status_regs.read('STATUS_3')
        d['status_7_dump'] = self._status_regs.read('STATUS_7')
        if self.mbed: 
            d.update({
                'mbed_vreg' : self.mbed.read_vreg(),
                'mbed_vbat' : self.mbed.read_vbat(),
                'mbed_vdev' : self.mbed.read_vdev(),
                'mbed_temperature' : self.mbed.read_temperature()
        })
        return d


    def __str__(self):
        """Returns string representation of TestChip object including key status information
        """
        res = "M0N0S2 Testchip (chip: {})".format(
                "NA" if not self._chip_id else "{:03d}".format(self._chip_id))
        res += "\nADP Connected: {}\nMBED Connected: {}".format(
            ("YES ("+self._adp_port+")") if 
                    (self._adp_sock and self._adp_port) else "NO",
            ("YES ("+self._mbed_port+")") 
                    if (self.mbed and self._mbed_port) else "NO")
        if self._adp_sock:
            res += "\nIn Reset: {}".format(
                    "YES" if self._ctrl_regs.read('CTRL_0',
                            bit_group='master_reset') else
                    "NO")
            remap =  self._status_regs.read('STATUS_7', bit_group='memory_remap')
            res += "\nMemory base mapped to "
            if remap == 0:
                res += "ROM"
            elif remap == 1:
                res += "DEVRAM"
            elif remap == 2:
                res += "CODERAM"
            else:
                raise ValueError("Invalid Memory Remap")
            perf = self.get_perf()
            dvfs = self.get_dvfs()
            perf_label = None
            if int(perf) in self._perf_label_lookup.values():
                for name, level in self._perf_label_lookup.items():
                    if level == perf:
                        perf_label = name 
                        break 
            res += "\nDEVE: {}".format(
                    self._status_regs.read('STATUS_7', bit_group='deve_core'))
            res += "\nCurrent DVFS: {}{} (HW ID: {})".format(
                    dvfs,
                    " ("+perf_label+")" if perf_label else "",
                    perf)
            res += "\nSoftware: {}".format(
                "None" if not self._sw_directory else self._sw_directory)
        return res

    def get_sanity(self):
        """Utility method for printing all of the status and control registers to check that registers can be read and to check system status
        """
        self._logger.info("Reading ctrl_regs for sanity...")
        res = "CTRL_REG:\n0x0: 0x{:08X}\n0x1: 0x{:08X}\n0x2: 0x{:08X}\n".format(
            self._ctrl_regs.read(0),
            self._ctrl_regs.read(1),
            self._ctrl_regs.read(2))
        res += "0x3: 0x{:08X}\n0x4: 0x{:08X}\n0x5: 0x{:08X}\n".format(
            self._ctrl_regs.read(3),
            self._ctrl_regs.read(4),
            self._ctrl_regs.read(5))
        self._logger.info("Reading status_regs for sanity...")
        res += "STATUS_REG:\n0x0: 0x{:08X}\n0x1: 0x{:08X}\n0x2: 0x{:08X}\n".format(
            self._status_regs.read(0),
            self._status_regs.read(1),
            self._status_regs.read(2))
        res += "0x3: 0x{:08X}\n0x4: 0x{:08X}\n0x5: 0x{:08X}\n0x7: 0x{:08X}".format(
            self._status_regs.read(3),
            self._status_regs.read(4),
            self._status_regs.read(5),
            self._status_regs.read(7))
        return res

    def _initialise(self):
        """TestChip initialisation before ADP connection. Includes setting up the registers models and setting their read and write drivers to the dummy drivers for basic code testing even without a real chip and ADP connection. 
        """
        self._logger.info("Reading ctrl_regs for sanity...")
        self._logger.info("Initialising M0N0S2")
        mem_map_path = os.path.join(*[
                'registers_models','M0N0S2','mem_map.map.yaml'])
        pcsm_reg_path = os.path.join(*[
                'registers_models','M0N0S2','pcsm.regs.yaml'])
        spi_reg_path = os.path.join(*[
                'registers_models','M0N0S2','spi.regs.yaml'])
        gpio_reg_path = os.path.join(*[
                'registers_models','M0N0S2','gpio.regs.yaml'])
        ctrl_reg_path = os.path.join(*[
                'registers_models','M0N0S2','control.regs.yaml'])
        status_reg_path = os.path.join(*[
                'registers_models','M0N0S2','status.regs.yaml'])
        self._mem_map = mem_map_lib.MemoryMap(mem_map_path,logger=self._logger)
        self._logger.info("Setting up register models")
        self._spi_regs = registers_model.Registers_Model(spi_reg_path,logger=self._logger)
        self._spi_regs.set_read_driver(self._dummy_drivers.dummy_read)
        self._spi_regs.set_write_driver(self._dummy_drivers.dummy_write)
        self._pcsm_regs = registers_model.Registers_Model(
                pcsm_reg_path,
                logger=self._logger)
        self._pcsm_regs.set_write_driver(self._dummy_drivers.dummy_write)
        self._ctrl_regs = registers_model.Registers_Model(
                ctrl_reg_path,
                logger=self._logger)
        self._ctrl_regs.set_read_driver(self._dummy_drivers.dummy_read)
        self._ctrl_regs.set_write_driver(self._dummy_drivers.dummy_write)
        self._status_regs = registers_model.Registers_Model(
                status_reg_path,
                logger=self._logger)
        self._status_regs.set_read_driver(self._dummy_drivers.dummy_read)
        self._gpio_regs = registers_model.Registers_Model(
                gpio_reg_path,
                logger=self._logger)
        self._gpio_regs.set_read_driver(self._dummy_drivers.dummy_read)
        self._gpio_regs.set_write_driver(self._dummy_drivers.dummy_write)
    
    def set_adp_tx_callbacks(self, callbacks):
        """Set callbacks functions for ADP transaction protocol. 
        
        :param callbacks: dictionary where the keys are the transaction name
                          and the values are the callback functions to call 
                          when a transaction of that name is received. 
        :type callbacks: dict
        """
        self._adp_sock.set_adp_tx_callbacks(callbacks)

    def reset_hold(self):
        """Holds the chip in reset
        """
        self._ctrl_regs.write_set('CTRL_0',0x1)
        self._logger.info("Reset held")

    def reset_release(self):
        """Releases the chip reset reset
        """
        self._ctrl_regs.write_clear('CTRL_0',0x1)
        self._logger.info("Reset released")

    def _spi_write_through_adp(self,data):
        """Makes M0N0 SPI write data via ADP (manipulating the M0N0 SPI registers)
        
        :param data: data to send via SPI
        :type data: int
        """
        self._spi_regs.write("data_write", data,read_device=True)
        self._spi_regs.write("command", 0x1, read_device=True)
        start = time.time()
        while (self._spi_regs.read("status") == 1):
            if (time.time() - start) > 2:
                raise SpiTimeoutError("SPI transaction timed out")
            continue

    def _adp_to_pcsm_write(self, address, value):
        """Writes to the M0N0 PCSM register via on-chip SPI
        
        :param address: PCSM register address to write to
        :type address: int
        :param data: The (24-bit) data to write to the PCSM register
        :type address: int
        """
        self._logger.debug("ADP to PCSM via SPI. A: 0x{:X}, D: 0x{:X}".format(
                address,
                value))
        # Create 3 data byte packets: 
        value = value & 0xFFFFFF  # Strip down to 24 bits
        byte1 = value >> 16       # MSB
        byte2 = value >> 8 & 0xFF
        byte3 = value & 0xFF      # LSB
        self._logger.debug("SPI Write. Addr: 0x{:06X} Data: "
                           "0x{:02X}_{:02X}_{:02X}".format(
                                   address,
                                   byte1,
                                   byte2,
                                   byte3
                                   ))
        # Save SPI control setting for restoring
        initial_spi_ctrl = self._spi_regs.read("control")
        self._spi_regs.write("control", 0xC0)
        # Write to PCSM
        self._spi_write_through_adp(address)
        # next three bytes are 24b control register
        # (msb first)
        self._spi_write_through_adp(byte1)
        self._spi_write_through_adp(byte2)
        self._spi_write_through_adp(byte3)
        # Restore SPI control settings
        self._spi_regs.write("control",initial_spi_ctrl)
    
    def measure_rtc(self, time_period_s=5):
        """Measures the RTC frequency and error from target 33 kHz frequency

        :param time_period_s: The time interval over which to estimate the 
                              RTC frequency. The larger the time period, the
                              smaller the errors due to ADP communication time
                              overhead.
        :type time_period_s: int
        """
        self._logger.info("Measuring RTC over {:0.1f} seconds...".format(
                time_period_s))
        rtc_s_before = self.get_rtc(unit='s')
        py_s_before = time.time()
        time.sleep(time_period_s)
        rtc_s_after = self.get_rtc(unit='s')
        py_diff = time.time() - py_s_before
        rtc_diff = rtc_s_after - rtc_s_before
        error_pc = ((py_diff - rtc_diff) / py_diff) * 100.0
        rtc_total_ticks = int(rtc_diff/(self.RTC_PERIOD_US/(1000.0*1000.0)))
        rtc_period = py_diff / rtc_total_ticks 
        rtc_freq = 1 / rtc_period 
        message = "RTC Frequency (Hz): {:0.1f} ".format(rtc_freq)
        message += "(py_diff: {:0.2f}s, rtc_diff: {:0.2f}s, ".format(
                py_diff, rtc_diff)
        message += "err: {:0.2f} %)".format(error_pc)
        self._logger.info(message)
        return {'python_interval_s' : py_diff, 'rtc_interval_s' : rtc_diff, 'error_pc' : error_pc,
                'rtc_period_s' : rtc_period, 'rtc_freq_hz' : rtc_freq }

    def measure_timed_shutdown(self, time_ms):
        """Puts the chip into timed shutdown (requires chip to be in 'wait for ADP' mode and CALL WFI testcase to be included in the embedded software) and measures the time. 

        :param time_ms: The time duration to go into timed shutdown for in milliseconds
        :type time_ms: int
        :return: Dictionary of results including the RTC frequency
        :rtype: dict
        """
        raise NotImplementedError("Requires further testing")
        # 1. derive rtc ticks
        rtc_ticks = int(time_ms * self.RTC_ONE_MS_TICKS)
        self._logger.info("RTC ticks: {:d}".format(rtc_ticks))
        # 2. set the RTC wakeup PCSM register
        msbs = (rtc_ticks>>24)&0x00FFFFFF;
        self._pcsm_regs.write('rtc_wkup1', msbs)
        lsbs = (rtc_ticks)&0x00FFFFFF;
        self._pcsm_regs.write('rtc_wkup0', lsbs)
        # 3. set deep sleep
        print("msbs: 0x{:0X}, lsbs: 0x{:0X}".format(msbs, lsbs))
        self._tcs.run_testcase("EN_D_SLEEP", wait_for_output=True)
        rtc_s_before = self.get_rtc(unit='s')
        py_s_before = time.time()
        # 4. Call WFI
        res = self._tcs.run_testcase(
                "CALL_WFI",
                wait_for_output="<S><t><a><r><t><i><n><g>",
                timeout=time_ms*1000*10)
        rtc_s_after = self.get_rtc(unit='s')
        py_diff = time.time() - py_s_before
        rtc_diff = rtc_s_after - rtc_s_before
        error_pc = ((py_diff - rtc_diff) / py_diff) * 100.0
        rtc_total_ticks = int(rtc_diff/(self.RTC_PERIOD_US/(1000.0*1000.0)))
        rtc_period = py_diff / rtc_total_ticks 
        rtc_freq = 1 / rtc_period 
        message = "RTC Frequency (Hz): {:0.1f} ".format(rtc_freq)
        message += "(py_diff: {:0.2f}s, rtc_diff: {:0.2f}s, ".format(
                py_diff, rtc_diff)
        message += "err: {:0.2f} %)".format(error_pc)
        print(res)
        self._logger.info(message)
        return {
                'python_interval_s' : py_diff,
                'rtc_interval_s' : rtc_diff,
                'error_pc' : error_pc,
                'rtc_period_s' : rtc_period,
                'rtc_freq_hz' : rtc_freq }

    def measure_rtc_timed_shutdown(self, time_period_s=5):
        raise NotImplementedError("Requires further testing")
        return
        self._logger.info("Measuring RTC over {:0.1f} seconds...".format(
                time_period_s))
        self._logger.warn("Chip must be in 'wait for ADP' mode and "\
                +" have correct testcases compiled")
        rtc_s_before = self.setup_timed_shutdown(time_period_s*1000)
        py_s_before = time.time()
        time.sleep(10000) # todo wait for printf
        rtc_s_after = self.get_rtc(unit='s')
        py_diff = time.time() - py_s_before
        rtc_diff = rtc_s_after - rtc_s_before
        error_pc = ((py_diff - rtc_diff) / py_diff) * 100.0
        rtc_total_ticks = int(rtc_diff/(self.RTC_PERIOD_US/(1000.0*1000.0)))
        rtc_period = py_diff / rtc_total_ticks 
        rtc_freq = 1 / rtc_period 
        message = "RTC Frequency (Hz): {:0.1f} ".format(rtc_freq)
        message += "(py_diff: {:0.2f}s, rtc_diff: {:0.2f}s, ".format(
                py_diff, rtc_diff)
        message += "err: {:0.2f} %)".format(error_pc)
        self._logger.info(message)
        return {'python_interval_s' : py_diff,
                'rtc_interval_s' : rtc_diff,
                'error_pc' : error_pc,
                'rtc_period_s' : rtc_period,
                'rtc_freq_hz' : rtc_freq }

    
    def derive_rtc_trim(self, time_period_s=5, recursion_depth=14):
        """ Derives the RTC trim value for active mode. The result is printed to screen. 

        :param time_period_s: The time duration over which to measure the RTC
                              frequency. The longer the duration, the lower
                              the error due to ADP timing overheads. 
        :type time_period_s: int
        :param recursion_depth: Binary search recursion depth
        :type recursion_depth: int
        """
        self._logger.info("Enabling RTC Forward Body Bias (FBB)")
        self._pcsm_regs.write('rtc_ctrl1', 1, bit_group='en_fbb')
        current_rtc_trim = self._pcsm_regs.read(
                'rtc_ctrl0',bit_group='trim_res_tune')
        self._logger.info("Starting RTC trim value: 0x{:08X}".format(current_rtc_trim))
        def rtc_trim_inner(trim_val, args):
            args['logger'].info("rtc_trim_inner")
            # Setting the trim
            self._pcsm_regs.write('rtc_ctrl0', trim_val, bit_group='trim_res_tune')
            # read the RTC
            self._logger.info("New Trim: 0x{:08X}".format(
                    trim_val))
            results = self.measure_rtc(time_period_s=time_period_s)
            if results['rtc_interval_s'] < results['python_interval_s']:
                results['bin_value'] = 1
            else:
                results['bin_value'] = 0
            return results
        args = {}
        args['logger'] = self._logger
        args['trim_min'] = 0
        args['trim_max'] = 2**9
        temp_res = utils.bin_search(
                args['trim_min'],
                args['trim_max'],
                recursion_depth,
                rtc_trim_inner,
                args,
                is_int=True)

    def derive_rtc_trim_timed_shutdown(self, time_period_s=5, recursion_depth=14):
        """ Derives the RTC trim value for timed-shutdown mode. The result is printed to screen. 

        :param time_period_s: The time duration over which to measure the RTC
                              frequency. The longer the duration, the lower the
                              error due to ADP timing overheads. 
        :type time_period_s: int
        :param recursion_depth: Binary search recursion depth
        :type recursion_depth: int
        """
        raise NotImplementedError("Requires testing")
        self._logger.info("Enabling RTC Forward Body Bias (FBB)")
        self._pcsm_regs.write('rtc_ctrl1', 1, bit_group='en_fbb')
        current_rtc_trim = self._pcsm_regs.read(
                'rtc_ctrl0',bit_group='trim_res_tune')
        self._logger.info("Starting RTC trim value: 0x{:08X}".format(current_rtc_trim))
        def rtc_trim_inner(trim_val, args):
            args['logger'].info("rtc_trim_inner")
            # Setting the trim
            self._pcsm_regs.write('rtc_ctrl0', trim_val, bit_group='trim_res_tune')
            # read the RTC
            self._logger.info("New Trim: 0x{:08X}".format(
                    trim_val))
            results = self.measure_rtc(time_period_s=time_period_s)
            if results['rtc_interval_s'] < results['python_interval_s']:
                results['bin_value'] = 1
            else:
                results['bin_value'] = 0
            return results
        args = {}
        args['logger'] = self._logger
        args['trim_min'] = 0
        args['trim_max'] = 2**9
        temp_res = utils.bin_search(
                args['trim_min'],
                args['trim_max'],
                recursion_depth,
                rtc_trim_inner,
                args,
                is_int=True)
     
    @staticmethod
    def setup_chip(params, logger, chip=None, skip_reload=False):
        """Creates a M0N0S2 chip instance using the parameters (includes command line options).

        :param params: Dictionary of parameters, including forwarded command line options
        :type params: dict
        :param logger: The logger to use for logging messages to console and file
        :type logger: logging.Logger object, optional
        :param chip: An old chip instance can be passed to safely close and
                     restart with a new object
        :type chip: testchip.M0N0S2, optional
        :param skip_reload: Bypasses the hex/binary write (usually to DEVRAM)
                            - assumes VDEV has not been reset and saves
                            experiment time
        :type skip_reload: Whether to skip reloading of the memory (i.e. 
                           if VDEV not reset)
        :return: The new chip object
        :rtype: testchip.M0N0S2 object
        """
        if chip:
            chip.exit()
            del chip
        if 'mbed_port' not in params:
            params['mbed_port'] = None
        if 'tcs_to_run_file' not in params:
            params['tcs_to_run_file'] = None
        if 'chip_name' not in params:
            params['chip_name'] = None
        chip = M0N0S2(
                logger=logger,
                mbed_port=params['mbed_port'],
                sw_directory=params['software'],
                load_devram=params['load_devram'],
                chip_id=int(params['chip_id']),
                tcs_to_run_path=params['tcs_to_run_file'],
                chip_name = params['chip_name'])
        if chip.mbed:
            chip.mbed.external_wake()
        else:
            if params['adp_port']:
                logger.warning("No MBED, apply EXTWAKE manually from board "
                               "switch")
                input("Press any key after applying EXTWAKE")
        if params['adp_port']:
            if params['adp_port'].lower() == 'auto':
                params['adp_port'] = utils.derive_adp_port(logger)
            chip.connect(
                adp_port=params['adp_port'],
                skip_reload=skip_reload,
                load_trims=params['load_trims'])
        else:
            logger.error("Not connected to chip. Can only test models")
        chip.reset_hold()
        logger.info("Created S2 Chip")
        logger.info("(Reset held)")
        if 'auto_release' in params:
            if params['auto_release']:
                chip.reset_release()
                logger.info("(Reset release automatically)")
        return chip

