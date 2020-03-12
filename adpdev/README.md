# ADP Dev

The ADP Development software is used for programming and debugging code on the M0N0 System via the ADP interface. 

## Requirements:

* Python 3.7 or higher
* pyserial
* pyyaml

## Setting up the Python Environment with Anaconda (optional)

[Conda](https://conda.io/en/latest/) can be used to manage the packages in a separate environment (multiple environments can be switched between with different versions of pythons and different libraries). 

Install [Anaconda](https://www.anaconda.com/distribution/) (Python 3.7)

(Note: when installing Anaconda on an Ubuntu virtual machine, a bug in Anaconda meant that the virtual machine needed to have more than one CPU core in order to install). 

Using [Conda](https://docs.conda.io/projects/conda/en/latest/user-guide/getting-started.html) from the terminal, create a Python 3 environment (i.e. called `adpenv`):
```console
conda create --name adpenv python=3.7.3
```

Activate the new `adpenv` environment:
```console
conda activate adpenv
```

Install the dependencies:
```console
conda install pyserial
conda install pyyaml
```

To be able to compile the documentation:
```console
conda install sphinx
```

**Note:** you must re-activate the environment every time you create a new terminal or restart etc.:
```console
conda activate adpenv  
```

## ADPDev Script

The `adpdev.py` script is used to connect to the chip via ADP, setup the chip (including optional loading of trim values), load software to the chip, run software, and provide debug features (such as reading/writing to memory and registers). 

To see usage information:

```console
./adpdev.py -h
```

## Basic Testing

First, connect the DevModule, or, if already connected, reset VBAT by holding the "Vbat OFF" button for two seconds. 

In order to load software onto M0N0:
```console
./adpdev.py --chip-id 34 --adp-port /dev/cu.usbserial-AI06BWW3 -s ../projects/hello_world/build/
```
where:

* `34` is the chip ID (written on the line under "M0N0-S2" on the top of the chip package)
* `../projects/hello_world/build/` is the path to the project build directory containing the `.bin` and, if using remote testcase control, the testcase list CSV file.  The project must be built using `make` first. 
* `--adp-port /dev/cu.usbserial-AI06BWW3` tells the software the name of the port with the connected ADP. 

In order to get a list of all connected serial ports, add `--show-ports` to the end of the command. 

When running the command, you will be prompted to issue the EXTWAKE. Press the EXTWAKE button on the DevHat board and press enter on the host computer to continue. 
The script then:

* Connects to the board via ADP
* Puts the M0N0 chip (specifically the Cortex-M33 MCU) into reset mode ("reset hold")
* Loads the `.bin` file in the specified `build` directory into M0N0's Development RAM
* Remaps the memory so that M0N0 will execute from the Development RAM instead of the ROM
* Provides an interactive prompt where commands can be issued. 

**Note:** When  you are left with an interactive prompt, the CPU is still in reset, allowing you to configure parts of the system before execution starts. 

To release the reset, simply type `chip.reset_release()` into the terminal. 

Or, if you want it to release the reset automatically each time, use the `--auto-release` flag, for example:
```console
./adpdev.py --chip-id 34 --adp-port /dev/cu.usbmodem205A389B474B1 -s ../projects/dvfs_example/build --auto-release
```

**Note:** Ensure that you turn VBAT off for ~3 seconds and back on again before loading a new program, to ensure that the PCSM state has been refreshed. 

### Stopping the Script

The script can be stopped by calling `close()`, `quit()` or using CTRL+C. 

### Reading Standard Out ("printf")

In order to read standard output as well as the raw result of other commands issued via ADP, continuously read the `logs/read_buffer.log-stripped.log`. For example, in Ubuntu or MacOS, create a new terminal window and execute:
```console
tail -f logs/read_buffer.log-stripped.log
```

To read _just_ the STDOUT ("printf"), read the `logs/read_buffer.log-stdout.log` instead. 

For Windows users without the `tail` command a simple python script has been created called `pytail`: 
```console
./silicon_libs/pytail.py -f logs/read_buffer.log-stdout.log
```

### Basic Commands

From the `adpdev.py` interactive prompt, several commands can be issued for testing. 

The M0N0 system is encapsulated in a software object that is stored in a variable named `chip`. 

One of the most basic commands is:
```python
>>> print(chip)
```
which gives basic status information about the chip (some of the settings, such as the `In Reset`, `DEVE`, `Memory base mapped to DEVRAM` and `Current perf` are read from the chip directly when this command is called. Example output:
```
M0N0S2 Testchip (chip: 034)
ADP Connected: YES (/dev/cu.usbserial-AI06BWW3)
MBED Connected: NO
In Reset: YES
Memory base mapped to DEVRAM
DEVE: 1
Current perf: 25
Software: ../projects/hello_world/build/
```

A basic method of reading important control and status registers is by using the `get_sanity` method:
```python
print(chip.get_sanity())
```

Note that a warning regarding a `READ MISMATCH` for `Addr: 0x2` is expected, as this control register is updated automatically, due to dynamic ROM banking. 
Example output:
```
INFO (testchip.py:524):  Reading ctrl_regs for sanity...
WARNING (registers_model.py:704):  READ MISMATCH registers_models/M0N0S2/control.regs.yaml: Addr: 0x2 (0xF0000008) device: 0x80078007, model: 0x0
INFO (testchip.py:533):  Reading status_regs for sanity...
CTRL_REG:
0x0: 0x00000001
0x1: 0x0007FFF0
0x2: 0x80078007
0x3: 0x55AA0000
0x4: 0x00000225
0x5: 0x00000000
STATUS_REG:
0x0: 0x00000000
0x1: 0x00000010
0x2: 0x00A4E5FB
0x3: 0x00100010
0x4: 0x00000000
0x5: 0x00000000
0x7: 0x0101479B
```

The current DVFS level can be read using:
```python
chip.get_dvfs()
```

Example output:
```
5
```

and set using (passing a number in the range 0-15):
```python
chip.set_dvfs(3)
```

## System Commands


## Reading and Writing Registers

The M0N0 system registers are accessed through the objects:

* `ctrl_regs` - The control registers
* `status_regs` - The (read only) status registers
* `pcsm` - The (write only) PCSM registers (although some values are mapped to a status register)
* `spi` - The SPI registers

To see the names, PoR (power-on-reset) value, and _modelled_ current values, e.g.:
```python
print(chip.pcsm)
```

The register names match those in the official documentation. 

To read a whole register, e.g.:
```python
chip.status_regs.read("STATUS_2")
```

To read a "bit-group" (or "bit-range") of a register:
```python
chip.status_regs.read("STATUS_7", bit_group="real_time_flag")
```



Writing from registers is performed in a similar manner. To write to a whole register:
```python
chip.ctrl_regs.write("CTRL_4", 0x12)
```

Or, as is more common, to write to a specific bit-group of a register:
```python
chip.ctrl_regs.write("CTRL_4", 0x8, bit_group="dataram_delay")
```

## Reading and Writing Memory

Access to the registers (such as the control registers, status registers, PCSM registers, SPI registers etc.) should be through the specific register objects (see above). 


To read any memory location in the memory map:
```python
chip.adp_sock.memory_read(0x0)
```

To write any memory location in the memory map:
```python
chip.adp_sock.memory_write(0x0, 0x2)
```
where the first parameter is the address and the second is the data to write. 


To read, say, 16 words (64 bytes) of a memory region:
```python
chip.read_memory_region("DEVRAM", words=16)
```
(returns a list of integers representing each word). 

The name of the region, i.e. "DEVRAM" corresponds to "Name" in the memory map table of the documentation.

Alternatively, simply use:
```python
print(chip.mem_map)
```
To see the memory map. 


To save the result to a file (written as hexadecimal):
```python
chip.read_memory_region("DEVRAM", words=16, save_to_file="temp_file.hex")
```

Sample of `temp_file.hex`:
```
0x20003FF0
0x000022FD
0x0000232F
0x00001D59
0x00001D99
0x00001DC1
0x00001DD5
0x00000000
0x00000000
0x00000000
0x00000000
0x0000232F
0x0000232F
0x00000000
0x0000232F
0x00001DE9
```

To read a full memory region (*NOTE:* this can take over 10 minutes):
```python
chip.read_memory_region("DEVRAM")
```


## Running Embedded Functions ("Testcases")


### Measure the RTC Frequency

The RTC can be measured by comparing the ADPDev Python time over a duration. 

```python
chip.measure_rtc()
```

To override the default time period (in seconds) it is measured over:
```python
chip.measure_rtc(time_period_s=30)
```

The larger the time period, the lower the error due to communication timing. 

Example output:
```
INFO (testchip.py:688):  Measuring RTC over 30.0 seconds...
INFO (testchip.py:703):  RTC Frequency (Hz): 33114.1 (py_diff: 30.38s, rtc_diff: 30.48s, err: -0.35 %)
```


```
INFO (testchip.py:699):  py_diff: 10.41s, rtc_diff: 10.49s, err: -0.76 %
```

### Trimming the RTC 

As the RTC frequency can be measured from the Python scripts, it is possible to derive the trim values. 
The method uses a binary search and previously described `measure_rtc`. 

```python
chip.derive_rtc_trim()
```

To specify the time period of the RTC measurement:

```python
chip.derive_rtc_trim(time_period_s=30)
```


Once the trim value has been found, it can manually be added to the trims database:
```
../../adpdev/silicon_libs/trims/M0N0S2/trims.yaml
```

For example, if the function was run on `chip 007` and the RTC trim value was found to be `0x000001F4`:
```
INFO (testchip.py:724):  New Trim: 0x000001F1
INFO (testchip.py:688):  Measuring RTC over 5.0 seconds...
INFO (testchip.py:703):  RTC Frequency (Hz): 32705.9 (py_diff: 5.43s, rtc_diff: 5.38s, err: 0.89 %)
INFO (utils.py:23):  Converged
```

Add the following to the trim database (ensuring previous entries for that chip are removed):
```yaml
  7:
    pcsm:
      rtc_ctrl1: 
        en_fbb: 1
      rtc_ctrl0:
        trim_res_tune: 0x1F4
```

**NOTE:** The `rtc_ctrl1 -> en_fbb` must be set to 1 as shown. 

**NOTE:** Do not put leading zeros on the chip ID as this could be interpreted as octal. 

## Misc

### VBAT must be reset if KWS has run

If the system is not intercepted by the ADPDev scripts early enough (i.e. if VBAT cycle and EXTWAKE were issued many _before_ running the scripta), then M0N0 (running from ROM) will start executing KWS, which turns on a feature called _autosampling_. Using the ADPDev to write to the PCSM or use SPI after autosampling is enabled can result in putting the SPI hardware into an undefined state. 

A key hint that this has occurred is seeing the warning after startup:
```
WARNING (registers_model.py:704):  READ MISMATCH registers_models/M0N0S2/spi.regs.yaml: Addr: 0x5 (0xB8000014) device: 0x8FA0, model: 0x8700
```
Which indicates the SPI registers are not in the state that the ADPDev scripts expected. 

To resolve this, simply reset VBAT by turning the switch off for 3 seconds before turning it on again. 

## Reference Manual

A simple reference manual can be generated. See the [docs README.md](docs/README.md). 


