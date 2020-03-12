PROJECT = m0n0

# *****************************************************************************
TOP_PATH := $(abspath ../../)
MAKEFILE_NAME		:= $(MAKEFILE_LIST)
MAKEFILE_DIR		:= $(abspath $(dir $(MAKEFILE_NAME) ) )
M0N0_SYSTEM_DIR		:= $(abspath ../../M0N0_libs/M0N0_system )
M0N0_PRINTF_DIR		:= $(abspath ../../M0N0_libs/M0N0_printf )
M0N0_TEST_UTIL_DIR  :=  $(abspath ../../M0N0_libs/M0N0_test_util )
TC_LIST  :=  $(abspath ../../M0N0_libs/M0N0_test_util/testcase_list.csv )
CMSIS_5_DIR		:= $(abspath ../../CMSIS_5 )
DIST_DIR		:= $(abspath build )
BUILD_DIR		:= build_local
BUILD_TARGET		= m0n0

MKDIR_P = mkdir -p

GNU_PREFIX = arm-none-eabi

# *****************************************************************************
# Toolchain setup
# *****************************************************************************
AS       = $(GNU_PREFIX)-as
CC       = $(GNU_PREFIX)-gcc
CPP      = $(GNU_PREFIX)-g++
LD       = $(GNU_PREFIX)-g++
OBJCOPY  = $(GNU_PREFIX)-objcopy
OBJDUMP  = $(GNU_PREFIX)-objdump
SIZE     = $(GNU_PREFIX)-size

# *****************************************************************************
# Compiler and linker setup -march=armv8-m.main+dsp
# *****************************************************************************

LINKER_FILE = m0n0.ld
LINKER_PATH = $(M0N0_SYSTEM_DIR)/lkr/$(LINKER_FILE)
# armv8-m.main+dsp+nofp
##################
# Main CPU has no floating point unit.
ASMFLAGS = -x assembler-with-cpp -mcpu=cortex-m33 -march=armv8-m.main+dsp -mthumb -mno-unaligned-access
##################
# NO DSP
##################
# CCFLAGS  = -mcpu=cortex-m33+nodsp -march=armv8-m.main -mtune=cortex-m33+nodsp -mthumb
##################
# DSP
# Main CPU has no floating point unit.
CCFLAGS  = -mcpu=cortex-m33 -march=armv8-m.main+dsp -mtune=cortex-m33 -mthumb
##################
CCFLAGS += -mfloat-abi=soft
# Don’t recognize built-in func that do not begin with ‘__builtin_’ as prefix
CCFLAGS += -fno-builtin
# no unaligned access improves performance, otherwise mem accesses
# that cross 32-bit boundaries will result in 2 transactions
CCFLAGS += -mno-unaligned-access -mlong-calls
CCFLAGS += -fno-inline-functions
# warnings
CCFLAGS += -Wall -Werror
CCFLAGS += -Wduplicated-cond
CCFLAGS += -Wduplicated-branches
CCFLAGS += -Wshadow
CCFLAGS += -Wextra
# CCFLAGS += -Wcast-qual
# debug and/or optimizations
# CCFLAGS += -g3 -Og
# Other optimizations
CCFLAGS += -Os
##################################################
# WARNING
# either use -O3 with fno-inline-functions or -O2
##################################################
# CCFLAGS += -O2
CCFLAGS += -O3
# verbose
CCFLAGS += -v
# To keep the data in separate data sections
# works together with
CCFLAGS += -fdata-sections
# to keep functions in separate sections, so they (data and functions) can be discarded if unused
CCFLAGS += -ffunction-sections
# strict aliasing with O2 or O3 flags
# http://thiemonagel.de/2010/01/no-strict-aliasing/
CCFLAGS += -fno-strict-aliasing
CCFLAGS += --short-enums
####################################################
####################################################
# libs
CCFLAGS += --specs=nosys.specs
# Nano to reduce binary size.
CCFLAGS += --specs=nano.specs
# Clear BSS region on boot. Set main as entry point.
CCFLAGS += -D__STARTUP_CLEAR_BSS=1 -D__START=main
# custom print
CCFLAGS += -DM0N0_PRINT=1
# m0n0_s2
CCFLAGS += -DM0N0_S2=1
CCFLAGS += -DARMCM33_DSP_FP -DCMSIS_device_header=\"ARMCM33_DSP_FP.h\" 
CPPFLAGS  = --std=gnu++11
CPPFLAGS  += -DDEFAULT_LOG_LEVEL=DEBUG
CPPFLAGS  += -DEXTRA_CHECKS
#CPPFLAGS  += -DSUPPRESS_STDOUT

# Put functions in separate sections so unused ones may be removed.
LDFLAGS += -Xlinker -Map=$(BUILD_DIR)/$(BUILD_TARGET)_orig_map.map
# NOTE: mthumb and mcpu must both be set to link correctly with the v8m libraries.
LDFLAGS += -T$(LINKER_PATH) -mcpu=cortex-m33 -mthumb -Wl,--gc-sections
# LDFLAGS += -flto
# no unaligned access improves performance, otherwise mem accesses
# that cross 32-bit boundaries will result in 2 transactions
LDFLAGS += -mno-unaligned-access
LDFLAGS += -lc
# Use nosys as we have no OS.
LDFLAGS += --specs=nosys.specs
# Nano to reduce binary size.
LDFLAGS += --specs=nano.specs
LDFLAGS += -lnosys
LDFLAGS += -nostartfiles
# verbose
# LDFLAGS += -v
# entry point and clear bss
LDFLAGS += -D__STARTUP_CLEAR_BSS=1
LDFLAGS += -D__START=main
LDFLAGS += --std=gnu++11
#*****************************************************************************
# Target files
#*****************************************************************************

# Header files

# M0n0 specific
M0N0_INCPATH += -I$(M0N0_SYSTEM_DIR)/include
M0N0_INCPATH += -I$(M0N0_PRINTF_DIR)/include
M0N0_INCPATH += -I$(M0N0_TEST_UTIL_DIR)/include
INCLUDE_PATHS += $(M0N0_INCPATH)
# CMSIS
CMSIS_5_DEVICE_INCPATH += -I$(CMSIS_5_DIR)/Device/ARM/ARMCM33/Include
CMSIS_5_CORE_INCPATH   += -I$(CMSIS_5_DIR)/CMSIS/Core/Include
INCLUDE_PATHS += $(CMSIS_5_DEVICE_INCPATH)
INCLUDE_PATHS += $(CMSIS_5_CORE_INCPATH)
# PROJECT
PROJECT_INCPATH += -I$(MAKEFILE_DIR)/include
INCLUDE_PATHS += $(PROJECT_INCPATH)

##############################################################
# Additional libraries, like CMSIS_NN or CMSIS_DSP
##############################################################
# include  .../list_of_additional_libraries/libraries.mk
# ...
# 	CMSIS_5_DSP_INCPATH    += -I$(CMSIS_5_DIR)/CMSIS/DSP/Include
# 	CMSIS_5_NN_INCPATH     += -I$(CMSIS_5_DIR)/CMSIS/NN/Include
# 	INCLUDE_PATHS += $(CMSIS_5_DSP_INCPATH)
# 	INCLUDE_PATHS += $(CMSIS_5_NN_INCPATH)
# ...

# C source files
# system
C_FILES += $(M0N0_SYSTEM_DIR)/src/system_m0n0.c
CPP_FILES += $(M0N0_SYSTEM_DIR)/src/m0n0.cpp
CPP_FILES += $(M0N0_SYSTEM_DIR)/src/sysutil.cpp
CPP_FILES += $(M0N0_TEST_UTIL_DIR)/src/tc_functions.cpp
# printf software components
C_FILES += $(M0N0_PRINTF_DIR)/src/m0n0_printf.c
##############################################################
# Additional libraries, like CMSIS_NN or CMSIS_DSP
##############################################################
# include  .../list_of_additional_src/user_src.mk
# ...
# 	C_FILES += $(MAKEFILE_DIR)/src/my_src.c
C_FILES += $(M0N0_SYSTEM_DIR)/src/m0n0_defs.c
C_FILES += $(M0N0_SYSTEM_DIR)/src/interrupts.c
CPP_FILES += $(MAKEFILE_DIR)/src/main.cpp

###############################################################################
# Non-worning objects
# Do not consider CMSIS_NN warnings as errors
# $(CMSIS_NN_SRC):=$(filter-out -Werror,$(CCFLAGS))
# Uncomment if using DSP or NN libs
# 
# NON_WARNING_ERROR_FILES = $(CMSIS_NN_SRC) $(CUSTOM_MFCC_SRC) $(CMSIS_DSP_SRC)
# NON_WARNING_ERROR_F_NAMES = $(notdir $(NON_WARNING_ERROR_FILES))
# NON_WARNING_ERROR_PATHS = $(dir $(NON_WARNING_ERROR_FILES) )
# NON_WARNING_ERROR_OBJECTS = $(addprefix $(BUILD_DIR)/, $(NON_WARNING_ERROR_F_NAMES:.c=.o) )
# $(NON_WARNING_ERROR_OBJECTS): CCFLAGS:=$(filter-out -Werror,$(CCFLAGS))
# vpath %.c $(NON_WARNING_ERROR_PATHS)
################################################################################

# ASM files
ASM_FILES  = $(M0N0_SYSTEM_DIR)/src/GCC/startup_m0n0.S
##############################################################
# Additional libraries, like CMSIS_NN or CMSIS_DSP
##############################################################
# include  .../list_of_additional_src/user_src.mk
# ...
# 	ASM_FILES += $(MAKEFILE_DIR)/src/my_ASM.S

# Strip dirs to create object names
C_FILE_NAMES = $(notdir $(C_FILES))
CPP_FILE_NAMES = $(notdir $(CPP_FILES))
C_PATHS = $(dir $(C_FILES) )
CPP_PATHS = $(dir $(CPP_FILES) )
C_OBJECTS = $(addprefix $(BUILD_DIR)/, $(C_FILE_NAMES:.c=.o) )
C_OBJECTS += $(addprefix $(BUILD_DIR)/, $(CPP_FILE_NAMES:.cpp=.o) )

ASM_FILE_NAMES = $(notdir $(ASM_FILES))
ASM_PATHS = $(dir $(ASM_FILES) )
ASM_OBJECTS = $(addprefix $(BUILD_DIR)/, $(ASM_FILE_NAMES:.S=.o) )

vpath %.c $(C_PATHS)
vpath %.cpp $(CPP_PATHS)
vpath %.S $(ASM_PATHS)

PROJECT_OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

V=@
ifeq ($(VERBOSE),1)
    V=
endif

.PHONY: directories
.PHONY: clean
.PHONY: all

all: directories clean build $(BUILD_TARGET).rcf $(BUILD_TARGET).map printsize distribute
# power_tcs: CCFLAGS += -DPOWER_TCS_ONLY -DTESTS_ONLY
# power_tcs: directories clean build $(BUILD_TARGET).rcf $(BUILD_TARGET).map printsize distribute
# tests_only: CCFLAGS +=  -DTESTS_ONLY
# tests_only: directories clean build $(BUILD_TARGET).rcf $(BUILD_TARGET).map printsize distribute

directories: ${DIST_DIR} ${BUILD_DIR}

${DIST_DIR}:
	${MKDIR_P} ${DIST_DIR}

${BUILD_DIR}:
	${MKDIR_P} ${BUILD_DIR}

build:
	@echo ""
	@echo ""
	@echo "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *"
	@echo "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *"
	@echo ""
	@echo ""

clean:
	rm -f $(PROJECT_OBJECTS) $(BUILD_DIR)/* $(DIST_DIR)/*

$(BUILD_DIR)/%.o: %.S
	@echo "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *"
	$(V)echo Assembling file: $(notdir $<)
	$(V)$(CPP) $(ASMFLAGS) $(INCLUDE_PATHS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c
	echo $(NON_WARNING_ERROR_OBJECTS)
	$(V)echo Compiling file using gcc: $(notdir $<)
	$(V)$(CC) $(CCFLAGS) --std=gnu11 $(INCLUDE_PATHS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cpp
	echo $(NON_WARNING_ERROR_OBJECTS)
	$(V)echo Compiling file using ++: $(notdir $<)
	$(V)$(CPP) $(CCFLAGS) $(CPPFLAGS) $(INCLUDE_PATHS) -c -o $@ $<

$(BUILD_TARGET).elf: $(PROJECT_OBJECTS)
	@echo "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *"
	$(V)echo Linking target: $@
	$(V)$(LD) $(LDFLAGS) $(LIBRARY_PATHS) $(PROJECT_OBJECTS) -lm -o $(BUILD_DIR)/$@
	$(V)echo Disassembling target: $@
	$(V)$(OBJDUMP) -S -d $(BUILD_DIR)/$@ > $(BUILD_DIR)/$(BUILD_TARGET).s

$(BUILD_TARGET).bin: $(BUILD_TARGET).elf
	$(V)echo Creating binary: $@
	$(V)$(OBJCOPY) -O binary $(BUILD_DIR)/$< $(BUILD_DIR)/$@

$(BUILD_TARGET).rcf: $(BUILD_TARGET).bin
	xxd -b -c4 $(BUILD_DIR)/$< | awk '{print $$5$$4$$3$$2;}' > $(BUILD_DIR)/$@
	xxd -c4 -g1 $(BUILD_DIR)/$< | awk '{print i$$5$$4$$3$$2}' > $(BUILD_DIR)/out.hex32

# use sed to remove leading spaces from GCC map file
$(BUILD_TARGET).map: $(BUILD_TARGET).elf
	nm $(BUILD_DIR)/$< > $(BUILD_DIR)/$@
	sed -e '/         /d' $(BUILD_DIR)/$@ > $(BUILD_DIR)/$@1
	mv $(BUILD_DIR)/$@1 $(BUILD_DIR)/$@

printsize:
	@echo "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *"
	$(V)$(SIZE) $(BUILD_DIR)/$(BUILD_TARGET).elf
	@echo "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *"

distribute:
	@echo Made $@
	cp $(BUILD_DIR)/$(BUILD_TARGET).map $(DIST_DIR)/
	cp $(BUILD_DIR)/$(BUILD_TARGET).bin $(DIST_DIR)/
	cp $(BUILD_DIR)/$(BUILD_TARGET).rcf $(DIST_DIR)/
	cp $(BUILD_DIR)/$(BUILD_TARGET).s   $(DIST_DIR)/
	cp $(TC_LIST)   $(DIST_DIR)/

