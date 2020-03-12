# M0N0 Library and Examples

## Introduction

This software supports software development and debugging on the M0N0 system developed within Arm Research. 

This software is tested on Ubuntu and MacOS, but should work on other Unixes. 

## Setup Instructions

The M0N0 software environment uses the [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm). 
See the [GNU-RM Downloads page](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). 

The M0N0 software environment also requires the [CMSIS Version 5](https://developer.arm.com/tools-and-software/embedded/cmsis). 
The M0N0 software environment was tested with [CMSIS 5.5.1 Release](https://github.com/ARM-software/CMSIS_5/releases/tag/5.5.1).

Detailed setup instructions are available at the GNU Arm Embedded Toolchain and CMSIS websites listed above. 

For convenience, an example of setting up the environment on Ubuntu is provided below. 


### Ubuntu Environment Setup Example

Install the prerequisites:
```console
sudo apt install make
sudo apt install git
```

The M0N0 environment was developed and tested with:
```console
gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2
```
which is available from the [GNU-RM Downloads page](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). Go to `8-2019-q3-update` and use the `Linux 64-bit` button to download.  

In this example, a directory named `gcc` was created in the `/home/USER` directory and the download extracted in there. 

Extract:
```console
tar -xjvf gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2
```

Navigate to a directory you want to install the M0N0 software, e.g.:
```console
cd
```

If not already done so, clone the [m0n0-library-and-examples](https://github.com/arm-software/m0n0-library-and-examples) (this repo):
```console
git clone https://github.com/arm-software/m0n0-library-and-examples
cd m0n0-library-and-examples
```

Edit the `GCC_INSTALL_DIR` in the `setenv.sh` file to add the path to the downloaded `GCC ARM` install. 

For example:
```console
GCC_INSTALL_DIR=/home/$USER/gcc/gcc-arm-none-eabi-8-2019-q3-update 
```

Download [CMSIS](https://developer.arm.com/tools-and-software/embedded/cmsis) 
from the [CMSIS GitHub page](https://github.com/ARM-software/CMSIS_5). 

This software was tested with the [CMSIS 5.5.1 release](https://github.com/ARM-software/CMSIS_5/releases/tag/5.5.1). 
```console
cd m0n0-library-and-examples
wget https://github.com/ARM-software/CMSIS_5/archive/5.5.1.tar.gz
tar -zxvf 5.5.1.tar.gz
mv CMSIS_5-5.5.1/ CMSIS_5
```

## Compiling a project

From the `m0n0-library-and-examples` directory, source the `setenv.sh` file:
```console
source setenv.sh
```

Then navigate to a project and build it using the `make` command, for example::
```console
cd projects/hello_world
make
```

The `build` directory should now contain a file ending in `.bin` which can be loaded onto the M0N0 system using the `adpdev` tools. 

By default, the required binary is called `m0n0.bin` (NOT the `devram.bin` file). 

### Preprocessor Flags

There are several preprocessor flags set in the Makefile:

* `-DDEFAULT_LOG_LEVEL=DEBUG` Sets the default log level for the M0N0 system log messages
* `-DEXTRA_CHECKS` Applies extra run-time tests (for example, will check that a control register address to read from is in the address range of the control register). Expected to be on for initial development but switched off for later development and deployment. 
* `-DSUPPRESS_STDOUT` Prevents any standard output (printf). This flag is usually omitted when testing with ADP connected but must be specified if ADP is not present with the DevModule to avoid the standard output buffer from blocking system execution. 

## Example Applications

Several example and training applications are included in the `projects` directory:

* `hello_world` - a simple example where "hello world" is printed via STDOUT and the chip goes into a "wait for ADP" mode for 20 seconds where workloads (_testcases_) can be commanded to run from the ADPDev scripts. 
* `demo_board_example_*` project directories with this prefix are examples that can be run on the Demo Board (contains a microphone and 7-segment display for showcasing the Keyword Spotting [KWS] application). 
* `devhat_barebone_*` project directories with this prefix are simple "bare-bone" examples for demonstrating a single feature on the DevModule+DevHat board configuration. 
* `devhat_example_*` project directories with this prefix are examples that can be run on the DevModule+DevHat board configuration
* `devhat_training_*` project directories with this prefix are training exercises for use on the DevModule+Devhat board configuration

## Loading Software and Debugging 

The ADPDEV software allows software to be loaded onto the M0N0 system and provides several debugging capabilities (such as reading and writing registers and memory locations, and reading printf output). 

The ADPDEV software is located in the `adpdev` directory - see the [adpdev/README.md](adpdev/README.md) file for ADPDEV setup and usage instructions. 


## Reference Manual

A simple reference manual can be compiled from the `docs` directory, see the [docs README.md](docs/README.md). 


