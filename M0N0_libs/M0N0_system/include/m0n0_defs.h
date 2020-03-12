
/*
 * Copyright (c) 2020, Arm Limited
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef M0N0_DEFS_H
#define M0N0_DEFS_H

//#include "m0n0_specific.h"
#include <stdint.h> 
#include "ARMCM33_DSP_FP.h"
#include "system_ARMCM33.h"

#if defined ( __CC_ARM	 )
#if (__ARMCC_VERSION < 400000)
#else
#pragma import _printf_widthprec
#endif
#endif

#define STDOUT_BASE       0xB4200000
#define STDIN_BASE        0xB4300000

typedef struct
{
  __IO uint8_t WDATA;
  __IO uint8_t resv_1;
  __IO uint8_t resv_2;
  __IO uint8_t resv_3;
  __IO uint8_t RDATA;
  __IO uint8_t resv_5;
  __IO uint8_t resv_6;
  __IO uint8_t resv_7;
  __IO uint8_t STAT;
  __IO uint8_t resv_9;
  __IO uint8_t resv_10;
  __IO uint8_t resv_11;
  __IO uint8_t INT;
  __IO uint8_t resv_13;
  __IO uint8_t resv_14;
  __IO uint8_t resv_15;
} FIFO_Type;

#define STDOUT                    ((FIFO_Type*)   STDOUT_BASE)
#define STDIN                     ((FIFO_Type*)   STDIN_BASE)

// Interrupt Handlers:
typedef void (*Handler_Func)(void);

uint32_t M0N0_read(uint32_t address);
uint32_t M0N0_read_bit_group( // calculates shift at "run-time"
        uint32_t address,
        uint32_t mask);
uint32_t M0N0_read_mask_and_shift(
        uint32_t address,
        uint32_t shift,
        uint32_t mask);
void M0N0_write(uint32_t address, uint32_t data);
void M0N0_write_bit_group( // calculates shift at "run-time"
        uint32_t address,
        uint32_t mask,
        uint32_t data);
void M0N0_write_mask_and_shift(
        uint32_t address,
        uint32_t shift,
        uint32_t mask,
        uint32_t data);

uint8_t mask_to_shift(uint32_t mask);

char M0N0_read_stdin(void);
void M0N0_write_stdout(uint8_t data);
uint8_t M0N0_spi_write(uint8_t data);
uint8_t M0N0_is_deve(void);

typedef enum {
   DESELECT = 0,
   SS0 = 1,
   SS1 = 2,
   SS2 = 4,
   SS3 = 8
} SPI_SS_t;

typedef enum {
   RW = 0,
   R, // read only
   W, // write only
} MEM_RD_WR_t;

typedef enum {
   DEBUG = 0,
   INFO = 1,
   WARN = 2,
   ERROR = 3,
} LOG_LEVEL_t;


/* Auto-generated from registers_models */
/*REGISTERS_MODELS_START*/


// MEM_MAP
// Generated from: registers_models/M0N0S2/mem_map.map.yaml

#define MEM_MAP_REMAP_BASE                                0x00000000
#define MEM_MAP_REMAP_SIZE                                0x10000000
#define MEM_MAP_DEVRAM_BASE                               0x10000000
#define MEM_MAP_DEVRAM_SIZE                               0x00020000
#define MEM_MAP_DATARAM_BASE                              0x20000000
#define MEM_MAP_DATARAM_SIZE                              0x00004000
#define MEM_MAP_SHRAM_BASE                                0x30000000
#define MEM_MAP_SHRAM_SIZE                                0x00001000
#define MEM_MAP_GPIO_BASE                                 0x40000000
#define MEM_MAP_GPIO_SIZE                                 0x10000000
#define MEM_MAP_ROM_BASE                                  0x50000000
#define MEM_MAP_ROM_SIZE                                  0x00020000
#define MEM_MAP_CODERAM_BASE                              0x60000000
#define MEM_MAP_CODERAM_SIZE                              0x00002000
#define MEM_MAP_PERIPHERALS_BASE                          0xB0000000
#define MEM_MAP_PERIPHERALS_SIZE                          0x10000000
  #define MEM_MAP_PERIPHERALS_SPI_BASE                    0xB8000000
  #define MEM_MAP_PERIPHERALS_SPI_SIZE                    0x0000001C
  #define MEM_MAP_PERIPHERALS_STDOUT_BASE                 0xB4200000
  #define MEM_MAP_PERIPHERALS_STDOUT_SIZE                 0x00000010
  #define MEM_MAP_PERIPHERALS_STDIN_BASE                  0xB4300000
  #define MEM_MAP_PERIPHERALS_STDIN_SIZE                  0x00000010
  #define MEM_MAP_PERIPHERALS_AES_BASE                    0xBC000000
  #define MEM_MAP_PERIPHERALS_AES_SIZE                    0x00000038
#define MEM_MAP_CONTROL_REGS_BASE                         0xF0000000
#define MEM_MAP_CONTROL_REGS_SIZE                         0x10000000

// GPIO
// Generated from: registers_models/M0N0S2/gpio.regs.yaml

#define GPIO_BASE_ADDR                                    0x40000000
#define GPIO_SIZE                                         0x00000410
#define GPIO_DATA_REG                                     0x40000000
#define GPIO_DIRECTION_REG                                0x40000400
#define GPIO_INTERRUPT_REG                                0x40000410

// AES
// Generated from: registers_models/M0N0S2/aes.regs.yaml

#define AES_BASE_ADDR                                     0xBC000000
#define AES_SIZE                                          0x00000034
#define AES_DATA_0_REG                                    0xBC000000
#define AES_DATA_1_REG                                    0xBC000004
#define AES_DATA_2_REG                                    0xBC000008
#define AES_DATA_3_REG                                    0xBC00000C
#define AES_KEY_0_REG                                     0xBC000010
#define AES_KEY_1_REG                                     0xBC000014
#define AES_KEY_2_REG                                     0xBC000018
#define AES_KEY_3_REG                                     0xBC00001C
#define AES_KEY_4_REG                                     0xBC000020
#define AES_KEY_5_REG                                     0xBC000024
#define AES_KEY_6_REG                                     0xBC000028
#define AES_KEY_7_REG                                     0xBC00002C
#define AES_CONTROL_REG                                   0xBC000030
  #define AES_R12_START_BIT_SHIFT                         0x00000000
  #define AES_R12_START_BIT_MASK                          0x00000001
  #define AES_R12_IRQ_ENABLE_BIT_SHIFT                    0x00000001
  #define AES_R12_IRQ_ENABLE_BIT_MASK                     0x00000002
  #define AES_R12_IRQ_CLEAR_FLAG_BIT_SHIFT                0x00000002
  #define AES_R12_IRQ_CLEAR_FLAG_BIT_MASK                 0x00000004
  #define AES_R12_ENCRYPT_OR_DECRYPT_BIT_SHIFT            0x00000003
  #define AES_R12_ENCRYPT_OR_DECRYPT_BIT_MASK             0x00000008
#define AES_STATUS_REG                                    0xBC000034
  #define AES_R13_COMPLETION_FLAG_BIT_SHIFT               0x00000000
  #define AES_R13_COMPLETION_FLAG_BIT_MASK                0x00000001

// CONTROL
// Generated from: registers_models/M0N0S2/control.regs.yaml

#define CONTROL_BASE_ADDR                                 0xF0000000
#define CONTROL_SIZE                                      0x00000014
#define CONTROL_SET_OFFSET                                0x00001000
#define CONTROL_CLR_OFFSET                                0x00002000
#define CONTROL_CTRL_0_REG                                0xF0000000
  #define CONTROL_MASTER_RESET_BIT_SHIFT                  0x00000000
  #define CONTROL_MASTER_RESET_BIT_MASK                   0x00000001
#define CONTROL_CTRL_1_REG                                0xF0000004
  #define CONTROL_R01_RUN_BIST_BIT_SHIFT                  0x00000000
  #define CONTROL_R01_RUN_BIST_BIT_MASK                   0x00000001
  #define CONTROL_R01_RUN_ONCE_BIT_SHIFT                  0x00000001
  #define CONTROL_R01_RUN_ONCE_BIT_MASK                   0x00000002
  #define CONTROL_R01_RUN_RD_LOOP_BIT_SHIFT               0x00000002
  #define CONTROL_R01_RUN_RD_LOOP_BIT_MASK                0x00000004
#define CONTROL_CTRL_2_REG                                0xF0000008
  #define CONTROL_R02_ROM_PWR_STATE_BIT_SHIFT             0x00000000
  #define CONTROL_R02_ROM_PWR_STATE_BIT_MASK              0x0000FFFF
  #define CONTROL_R02_ROM_ISO_STATE_BIT_SHIFT             0x00000010
  #define CONTROL_R02_ROM_ISO_STATE_BIT_MASK              0xFFFF0000
#define CONTROL_CTRL_3_REG                                0xF000000C
  #define CONTROL_R03_RUN_CODERAM_BIT_SHIFT               0x0000000A
  #define CONTROL_R03_RUN_CODERAM_BIT_MASK                0x00000400
  #define CONTROL_R03_RUN_SHRAM_BIT_SHIFT                 0x0000000B
  #define CONTROL_R03_RUN_SHRAM_BIT_MASK                  0x00000800
  #define CONTROL_R03_RUN_DATARAM_BIT_SHIFT               0x0000000C
  #define CONTROL_R03_RUN_DATARAM_BIT_MASK                0x00001000
  #define CONTROL_R03_RUN_ONCE_BIT_SHIFT                  0x0000000D
  #define CONTROL_R03_RUN_ONCE_BIT_MASK                   0x00002000
  #define CONTROL_R03_WR_LOOP_BIT_SHIFT                   0x0000000E
  #define CONTROL_R03_WR_LOOP_BIT_MASK                    0x00004000
  #define CONTROL_R03_RD_LOOP_BIT_SHIFT                   0x0000000F
  #define CONTROL_R03_RD_LOOP_BIT_MASK                    0x00008000
  #define CONTROL_R03_HALF_PATTERN_BIT_SHIFT              0x00000010
  #define CONTROL_R03_HALF_PATTERN_BIT_MASK               0xFFFF0000
#define CONTROL_CTRL_4_REG                                0xF0000010
  #define CONTROL_R04_SHRAM_DELAY_BIT_SHIFT               0x00000000
  #define CONTROL_R04_SHRAM_DELAY_BIT_MASK                0x0000000F
  #define CONTROL_R04_DATARAM_DELAY_BIT_SHIFT             0x00000004
  #define CONTROL_R04_DATARAM_DELAY_BIT_MASK              0x000000F0
  #define CONTROL_R04_CODERAM_DELAY_BIT_SHIFT             0x00000008
  #define CONTROL_R04_CODERAM_DELAY_BIT_MASK              0x00000F00
#define CONTROL_CTRL_5_REG                                0xF0000014
  #define CONTROL_R05_STROBE_BIT_SHIFT                    0x00000000
  #define CONTROL_R05_STROBE_BIT_MASK                     0x00000001
  #define CONTROL_R05_TESTCASE_ID_BIT_SHIFT               0x00000008
  #define CONTROL_R05_TESTCASE_ID_BIT_MASK                0x0000FF00
  #define CONTROL_R05_RTC_REPEAT_BIT_SHIFT                0x00000010
  #define CONTROL_R05_RTC_REPEAT_BIT_MASK                 0xFFFF0000

// SPI
// Generated from: registers_models/M0N0S2/spi.regs.yaml

#define SPI_BASE_ADDR                                     0xB8000000
#define SPI_SIZE                                          0x00000018
#define SPI_STATUS_REG                                    0xB8000000
#define SPI_COMMAND_REG                                   0xB8000004
  #define SPI_R01_COMMAND_BIT_SHIFT                       0x00000000
  #define SPI_R01_COMMAND_BIT_MASK                        0x00000003
#define SPI_DATA_WRITE_REG                                0xB8000008
#define SPI_DATA_READ_REG                                 0xB800000C
#define SPI_CLK_DIVIDE_REG                                0xB8000010
#define SPI_CONTROL_REG                                   0xB8000014
  #define SPI_R05_CLK_POLARITY_PHASE_BIT_SHIFT            0x00000000
  #define SPI_R05_CLK_POLARITY_PHASE_BIT_MASK             0x00000003
  #define SPI_R05_LSB_FIRST_BIT_SHIFT                     0x00000002
  #define SPI_R05_LSB_FIRST_BIT_MASK                      0x00000004
  #define SPI_R05_CHIP_SELECT_BIT_SHIFT                   0x00000003
  #define SPI_R05_CHIP_SELECT_BIT_MASK                    0x00000078
  #define SPI_R05_ENABLE_MASK_BIT_SHIFT                   0x00000007
  #define SPI_R05_ENABLE_MASK_BIT_MASK                    0x00000080
  #define SPI_R05_CS_ACTIVE_LOW_SS0_BIT_SHIFT             0x00000008
  #define SPI_R05_CS_ACTIVE_LOW_SS0_BIT_MASK              0x00000100
  #define SPI_R05_CS_ACTIVE_LOW_SS1_BIT_SHIFT             0x00000009
  #define SPI_R05_CS_ACTIVE_LOW_SS1_BIT_MASK              0x00000200
  #define SPI_R05_CS_ACTIVE_LOW_SS2_BIT_SHIFT             0x0000000A
  #define SPI_R05_CS_ACTIVE_LOW_SS2_BIT_MASK              0x00000400
  #define SPI_R05_ENABLE_AUTO_SAMPLE_BIT_SHIFT            0x0000000B
  #define SPI_R05_ENABLE_AUTO_SAMPLE_BIT_MASK             0x00000800
  #define SPI_R05_AUTO_SAMPLE_MODE_BIT_SHIFT              0x0000000C
  #define SPI_R05_AUTO_SAMPLE_MODE_BIT_MASK               0x00001000
  #define SPI_R05_ADC_BYTE_OFFSET_BIT_SHIFT               0x0000000D
  #define SPI_R05_ADC_BYTE_OFFSET_BIT_MASK                0x0000E000
#define SPI_SENSOR_DATA_REG                               0xB8000018

// STDIN
// Generated from: registers_models/M0N0S2/stdin.regs.yaml

#define STDIN_BASE_ADDR                                   0xB4300000
#define STDIN_SIZE                                        0x0000000C
#define STDIN_WDATA_REG                                   0xB4300000
  #define STDIN_WRITE_CHAR_BIT_SHIFT                      0x00000000
  #define STDIN_WRITE_CHAR_BIT_MASK                       0x000000FF
  #define STDIN_WAZ_BIT_SHIFT                             0x00000008
  #define STDIN_WAZ_BIT_MASK                              0xFFFFFF00
#define STDIN_RDATA_REG                                   0xB4300004
  #define STDIN_R01_READ_CHAR_BIT_SHIFT                   0x00000000
  #define STDIN_R01_READ_CHAR_BIT_MASK                    0x000000FF
  #define STDIN_R01_RAZ_BIT_SHIFT                         0x00000008
  #define STDIN_R01_RAZ_BIT_MASK                          0xFFFFFF00
#define STDIN_STATUS_REG                                  0xB4300008
  #define STDIN_R02_RXE_BIT_SHIFT                         0x00000000
  #define STDIN_R02_RXE_BIT_MASK                          0x00000001
  #define STDIN_R02_TXF_BIT_SHIFT                         0x00000001
  #define STDIN_R02_TXF_BIT_MASK                          0x00000002
  #define STDIN_R02_WAZ_RAZ_BIT_SHIFT                     0x00000002
  #define STDIN_R02_WAZ_RAZ_BIT_MASK                      0xFFFFFFFC
#define STDIN_INT_CTRL_REG                                0xB430000C
  #define STDIN_R03_INTERRUPT_ENABLE_BIT_SHIFT            0x00000000
  #define STDIN_R03_INTERRUPT_ENABLE_BIT_MASK             0x00000001
  #define STDIN_R03_FIFO_NOT_FULL_BIT_SHIFT               0x00000001
  #define STDIN_R03_FIFO_NOT_FULL_BIT_MASK                0x00000002
  #define STDIN_R03_WAZ_RAZ_BIT_SHIFT                     0x00000002
  #define STDIN_R03_WAZ_RAZ_BIT_MASK                      0xFFFFFFFC

// STATUS
// Generated from: registers_models/M0N0S2/status.regs.yaml

#define STATUS_BASE_ADDR                                  0xF0003000
#define STATUS_SIZE                                       0x0000001C
#define STATUS_STATUS_0_REG                               0xF0003000
#define STATUS_STATUS_1_REG                               0xF0003004
  #define STATUS_R01_BIST_COMPLETE_BIT_SHIFT              0x00000000
  #define STATUS_R01_BIST_COMPLETE_BIT_MASK               0x00000001
  #define STATUS_R01_BIST_PASS_BIT_SHIFT                  0x00000001
  #define STATUS_R01_BIST_PASS_BIT_MASK                   0x00000002
  #define STATUS_R01_BIST_FAIL_BIT_SHIFT                  0x00000002
  #define STATUS_R01_BIST_FAIL_BIT_MASK                   0x00000004
  #define STATUS_R01_BIST_RUNNING_BIT_SHIFT               0x00000003
  #define STATUS_R01_BIST_RUNNING_BIT_MASK                0x00000008
  #define STATUS_R01_BIST_LAST_PASS_BIT_SHIFT             0x00000004
  #define STATUS_R01_BIST_LAST_PASS_BIT_MASK              0x00000010
  #define STATUS_R01_BIST_ERRORS_BIT_SHIFT                0x00000005
  #define STATUS_R01_BIST_ERRORS_BIT_MASK                 0x00007FE0
#define STATUS_STATUS_2_REG                               0xF0003008
  #define STATUS_R02_RTC_LSBS_BIT_SHIFT                   0x00000000
  #define STATUS_R02_RTC_LSBS_BIT_MASK                    0xFFFFFFFF
#define STATUS_STATUS_3_REG                               0xF000300C
  #define STATUS_R03_BIST_EVER_COMPLETE_16KB_BIT_SHIFT    0x00000000
  #define STATUS_R03_BIST_EVER_COMPLETE_16KB_BIT_MASK     0x00000001
  #define STATUS_R03_BIST_EVER_PASSED_16KB_BIT_SHIFT      0x00000001
  #define STATUS_R03_BIST_EVER_PASSED_16KB_BIT_MASK       0x00000002
  #define STATUS_R03_BIST_EVER_FAILED_16KB_BIT_SHIFT      0x00000002
  #define STATUS_R03_BIST_EVER_FAILED_16KB_BIT_MASK       0x00000004
  #define STATUS_R03_BIST_RUNNING_16KB_BIT_SHIFT          0x00000003
  #define STATUS_R03_BIST_RUNNING_16KB_BIT_MASK           0x00000008
  #define STATUS_R03_BIST_LAST_PASS_16KB_BIT_SHIFT        0x00000004
  #define STATUS_R03_BIST_LAST_PASS_16KB_BIT_MASK         0x00000010
  #define STATUS_R03_BIST_ERRORS_16_KB_BIT_SHIFT          0x00000005
  #define STATUS_R03_BIST_ERRORS_16_KB_BIT_MASK           0x00007FE0
  #define STATUS_R03_CPU_SLEEPING_OUTPUT_BIT_SHIFT        0x0000000F
  #define STATUS_R03_CPU_SLEEPING_OUTPUT_BIT_MASK         0x00008000
  #define STATUS_R03_BIST_EVER_COMPLETE_8KB_BIT_SHIFT     0x00000010
  #define STATUS_R03_BIST_EVER_COMPLETE_8KB_BIT_MASK      0x00010000
  #define STATUS_R03_BIST_EVER_PASSED_8KB_BIT_SHIFT       0x00000011
  #define STATUS_R03_BIST_EVER_PASSED_8KB_BIT_MASK        0x00020000
  #define STATUS_R03_BIST_EVER_FAILED_8KB_BIT_SHIFT       0x00000012
  #define STATUS_R03_BIST_EVER_FAILED_8KB_BIT_MASK        0x00040000
  #define STATUS_R03_BIST_RUNNING_8KB_BIT_SHIFT           0x00000013
  #define STATUS_R03_BIST_RUNNING_8KB_BIT_MASK            0x00080000
  #define STATUS_R03_BIST_LAST_PASS_8KB_BIT_SHIFT         0x00000014
  #define STATUS_R03_BIST_LAST_PASS_8KB_BIT_MASK          0x00100000
  #define STATUS_R03_BIST_ERRORS_8KB_BIT_SHIFT            0x00000015
  #define STATUS_R03_BIST_ERRORS_8KB_BIT_MASK             0x7FE00000
  #define STATUS_R03_CPU_LOCKUP_OUTPUT_BIT_SHIFT          0x0000001F
  #define STATUS_R03_CPU_LOCKUP_OUTPUT_BIT_MASK           0x80000000
#define STATUS_STATUS_4_REG                               0xF0003010
  #define STATUS_R04_RTC_MSBS_BIT_SHIFT                   0x00000000
  #define STATUS_R04_RTC_MSBS_BIT_MASK                    0x00000FFF
#define STATUS_STATUS_5_REG                               0xF0003014
  #define STATUS_R05_PASS_BIT_SHIFT                       0x00000002
  #define STATUS_R05_PASS_BIT_MASK                        0x00000004
  #define STATUS_R05_FAIL_BIT_SHIFT                       0x00000003
  #define STATUS_R05_FAIL_BIT_MASK                        0x00000008
  #define STATUS_R05_COMPLETE_BIT_SHIFT                   0x00000004
  #define STATUS_R05_COMPLETE_BIT_MASK                    0x00000010
  #define STATUS_R05_RUNNING_BIT_SHIFT                    0x00000005
  #define STATUS_R05_RUNNING_BIT_MASK                     0x00000020
#define STATUS_STATUS_7_REG                               0xF000301C
  #define STATUS_R07_DEVE_CORE_BIT_SHIFT                  0x00000000
  #define STATUS_R07_DEVE_CORE_BIT_MASK                   0x00000001
  #define STATUS_R07_BATMON_REFRDY_BIT_SHIFT              0x00000001
  #define STATUS_R07_BATMON_REFRDY_BIT_MASK               0x00000002
  #define STATUS_R07_BATMON_UNDER_BIT_SHIFT               0x00000002
  #define STATUS_R07_BATMON_UNDER_BIT_MASK                0x00000004
  #define STATUS_R07_BATMON_OVER_BIT_SHIFT                0x00000003
  #define STATUS_R07_BATMON_OVER_BIT_MASK                 0x00000008
  #define STATUS_R07_PERF_BIT_SHIFT                       0x00000004
  #define STATUS_R07_PERF_BIT_MASK                        0x000001F0
  #define STATUS_R07_REAL_TIME_FLAG_BIT_SHIFT             0x0000000A
  #define STATUS_R07_REAL_TIME_FLAG_BIT_MASK              0x00000400
  #define STATUS_R07_EXT_WAKE_BIT_SHIFT                   0x0000000B
  #define STATUS_R07_EXT_WAKE_BIT_MASK                    0x00000800
  #define STATUS_R07_IO_CTRL_8_BIT_SHIFT                  0x0000000C
  #define STATUS_R07_IO_CTRL_8_BIT_MASK                   0x00001000
  #define STATUS_R07_MEMORY_REMAP_BIT_SHIFT               0x00000010
  #define STATUS_R07_MEMORY_REMAP_BIT_MASK                0x00030000
  #define STATUS_R07_ROM_WAKEUP_DELAY_BIT_SHIFT           0x00000013
  #define STATUS_R07_ROM_WAKEUP_DELAY_BIT_MASK            0x01F80000

// STDOUT
// Generated from: registers_models/M0N0S2/stdout.regs.yaml

#define STDOUT_BASE_ADDR                                  0xB4200000
#define STDOUT_SIZE                                       0x0000000C
#define STDOUT_WDATA_REG                                  0xB4200000
  #define STDOUT_WRITE_CHAR_BIT_SHIFT                     0x00000000
  #define STDOUT_WRITE_CHAR_BIT_MASK                      0x000000FF
  #define STDOUT_WAZ_BIT_SHIFT                            0x00000008
  #define STDOUT_WAZ_BIT_MASK                             0xFFFFFF00
#define STDOUT_RDATA_REG                                  0xB4200004
  #define STDOUT_R01_READ_CHAR_BIT_SHIFT                  0x00000000
  #define STDOUT_R01_READ_CHAR_BIT_MASK                   0x000000FF
  #define STDOUT_R01_RAZ_BIT_SHIFT                        0x00000008
  #define STDOUT_R01_RAZ_BIT_MASK                         0xFFFFFF00
#define STDOUT_STATUS_REG                                 0xB4200008
  #define STDOUT_R02_RXE_BIT_SHIFT                        0x00000000
  #define STDOUT_R02_RXE_BIT_MASK                         0x00000001
  #define STDOUT_R02_TXF_BIT_SHIFT                        0x00000001
  #define STDOUT_R02_TXF_BIT_MASK                         0x00000002
  #define STDOUT_R02_WAZ_RAZ_BIT_SHIFT                    0x00000002
  #define STDOUT_R02_WAZ_RAZ_BIT_MASK                     0xFFFFFFFC
#define STDOUT_INT_CTRL_REG                               0xB420000C
  #define STDOUT_R03_INTERRUPT_ENABLE_BIT_SHIFT           0x00000000
  #define STDOUT_R03_INTERRUPT_ENABLE_BIT_MASK            0x00000001
  #define STDOUT_R03_FIFO_NOT_FULL_BIT_SHIFT              0x00000001
  #define STDOUT_R03_FIFO_NOT_FULL_BIT_MASK               0x00000002
  #define STDOUT_R03_WAZ_RAZ_BIT_SHIFT                    0x00000002
  #define STDOUT_R03_WAZ_RAZ_BIT_MASK                     0xFFFFFFFC

// PCSM
// Generated from: registers_models/M0N0S2/pcsm.regs.yaml

#define PCSM_BASE_ADDR                                    0x000000
#define PCSM_SIZE                                         0x000023
#define PCSM_RTC_CTRL0_REG                                0x000001
  #define PCSM_R01_TRIM_LOCAL_REG_TUNE_BIT_SHIFT          0x000004
  #define PCSM_R01_TRIM_LOCAL_REG_TUNE_BIT_MASK           0x0003F0
  #define PCSM_R01_TRIM_CAP_TUNE_BIT_SHIFT                0x00000A
  #define PCSM_R01_TRIM_CAP_TUNE_BIT_MASK                 0x003C00
  #define PCSM_R01_TRIM_RES_TUNE_BIT_SHIFT                0x00000E
  #define PCSM_R01_TRIM_RES_TUNE_BIT_MASK                 0x7FC000
#define PCSM_RTC_CTRL1_REG                                0x000002
  #define PCSM_R02_EN_FBB_BIT_SHIFT                       0x000003
  #define PCSM_R02_EN_FBB_BIT_MASK                        0x000008
#define PCSM_MEM_CTRL_REG                                 0x000003
  #define PCSM_R03_SHRAM_RETXD_BIT_SHIFT                  0x000001
  #define PCSM_R03_SHRAM_RETXD_BIT_MASK                   0x000002
#define PCSM_IO_CTRL_REG                                  0x000004
  #define PCSM_R04_DS_VBAT_OUTBIDIR_BIT_SHIFT             0x000004
  #define PCSM_R04_DS_VBAT_OUTBIDIR_BIT_MASK              0x000030
  #define PCSM_R04_SR_VBAT_OUTBIDIR_BIT_SHIFT             0x000006
  #define PCSM_R04_SR_VBAT_OUTBIDIR_BIT_MASK              0x000040
  #define PCSM_R04_EXPOSE_PCSM_WRITE_BIT_SHIFT            0x000008
  #define PCSM_R04_EXPOSE_PCSM_WRITE_BIT_MASK             0x000100
  #define PCSM_R04_SPI_CS_POLARITY_BIT_SHIFT              0x000009
  #define PCSM_R04_SPI_CS_POLARITY_BIT_MASK               0x000E00
#define PCSM_RTC_WKUP0_REG                                0x000005
#define PCSM_RTC_WKUP1_REG                                0x000006
#define PCSM_AMSO_CTRL_REG                                0x000009
  #define PCSM_R09_TMUX_SEL_BIT_SHIFT                     0x000000
  #define PCSM_R09_TMUX_SEL_BIT_MASK                      0x0000FF
#define PCSM_BATMON_CTRL_REG                              0x00000B
  #define PCSM_R11_D_UVLO_COPY_BIT_SHIFT                  0x000000
  #define PCSM_R11_D_UVLO_COPY_BIT_MASK                   0x000003
  #define PCSM_R11_D_UVLO_LADDER_BIT_SHIFT                0x000002
  #define PCSM_R11_D_UVLO_LADDER_BIT_MASK                 0x00003C
#define PCSM_CODE_CTRL_REG                                0x00000C
  #define PCSM_R12_MEMORY_REMAP_BIT_SHIFT                 0x000000
  #define PCSM_R12_MEMORY_REMAP_BIT_MASK                  0x000003
  #define PCSM_R12_ROM_PWR_ON_DELAY_BIT_SHIFT             0x000003
  #define PCSM_R12_ROM_PWR_ON_DELAY_BIT_MASK              0x0001F8
#define PCSM_TCRO_CTRL_REG                                0x000011
#define PCSM_PERF_CTRL_REG                                0x00001B
  #define PCSM_R27_PERF_BIT_SHIFT                         0x000000
  #define PCSM_R27_PERF_BIT_MASK                          0x00001F
#define PCSM_DIG_PWR_EN_REG                               0x000020
  #define PCSM_R32_SHRAM_PERIPH_BIT_SHIFT                 0x000001
  #define PCSM_R32_SHRAM_PERIPH_BIT_MASK                  0x000002
  #define PCSM_R32_SHRAM_CORE_BIT_SHIFT                   0x000002
  #define PCSM_R32_SHRAM_CORE_BIT_MASK                    0x000004
  #define PCSM_R32_DATARAM_PERIPHERY_BIT_SHIFT            0x000003
  #define PCSM_R32_DATARAM_PERIPHERY_BIT_MASK             0x000078
  #define PCSM_R32_CODERAM_PERIPHERY_BIT_SHIFT            0x000007
  #define PCSM_R32_CODERAM_PERIPHERY_BIT_MASK             0x000180
  #define PCSM_R32_DATA_RAM_CORE_BIT_SHIFT                0x000009
  #define PCSM_R32_DATA_RAM_CORE_BIT_MASK                 0x001E00
  #define PCSM_R32_CODERAM_CORE_BIT_SHIFT                 0x00000D
  #define PCSM_R32_CODERAM_CORE_BIT_MASK                  0x006000
  #define PCSM_R32_ROM_BIT_SHIFT                          0x00000F
  #define PCSM_R32_ROM_BIT_MASK                           0x7F8000
#define PCSM_DIG_OUT_EN_REG                               0x000021
  #define PCSM_R33_SHRAM_ISO_BIT_SHIFT                    0x000001
  #define PCSM_R33_SHRAM_ISO_BIT_MASK                     0x000002
  #define PCSM_R33_DATARAM_ISO_BIT_SHIFT                  0x000003
  #define PCSM_R33_DATARAM_ISO_BIT_MASK                   0x000078
  #define PCSM_R33_CODERAM_ISO_BIT_SHIFT                  0x000007
  #define PCSM_R33_CODERAM_ISO_BIT_MASK                   0x000180
  #define PCSM_R33_ROM_ISO_BIT_SHIFT                      0x00000F
  #define PCSM_R33_ROM_ISO_BIT_MASK                       0x7F8000
#define PCSM_DIGPWR_AO_REG                                0x000022
#define PCSM_INTTIMER0_REG                                0x000023
/*REGISTERS_MODELS_END*/



#endif // M0N0_DEFS_H
