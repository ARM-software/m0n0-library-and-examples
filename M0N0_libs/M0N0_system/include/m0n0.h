
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
#ifndef M0N0_H
#define M0N0_H
#include <cstdint>
#include <string>
#include <cstdarg>
#include "sysutil.h"
#include "tc_functions.h" // for TC command via ADP

#include "ARMCM33_DSP_FP.h" // ARM v8 Main Line processor and core peripherals
#include "system_ARMCM33.h" // exectb_mcu System

extern "C" {
    #include "m0n0_defs.h"
}

// The M0N0_HEAP flag makes the M0N0_System object  instantiated
// on the heap with the "new" keyword. 
//#define M0N0_HEAP

/**
 * A class for supporting the M0N0 System features
 *
 * The M0N0_System class encapsulates the M0N0 System behaviour and provides 
 * an interface to the M0N0 System features in a singleton object. It is 
 * allocated on the stack by default but can be allocated on the heap using
 * by defining the M0N0_HEAP flag. 
 */
class M0N0_System {
#ifdef M0N0_HEAP
    /** Stores reference to M0N0 system object when using heap allocation
     *
     * I.e. only used if M0N0_HEAP flag is set
     */
    static M0N0_System* _instance;
#endif
    private:
        /**
         * Private constructor
         * 
         * Not to be called externally - externally exposed through get_sys
         */
        M0N0_System(); 
        /** Stores the system log level for log message filtering
         */
        LOG_LEVEL_t _log_level;
        /** 
         * Tasks to run before any shutdown (timed or deep)
         *
         * Executes any tasks that must be run before a shutdown to
         * ensure reliable operation (e.g. turning off autosampling, or
         * loading a different RTC trim value)
         */
        void _shutdown_cleanup(void);
        /**
         * A lookup table for converting DVFS level (0-15) to raw HW ID
         */
        uint8_t _perf_lookup[16] = {
            28, 24, 29, 20,
            30, 25, 31, 16,
            26, 21, 27, 22,
            17, 23, 18, 19};
        /**
         * A lookup table for converting raw HW ID to DVFS level (0-15)
         */
        uint8_t _inv_perf_lookup[32] = {
            128, 128, 128, 128, // 0-3
            128, 128, 128, 128, // 4-7
            128, 128, 128, 128, // 8-11
            128, 128, 128, 128, // 12-15
            7  , 12 , 14 , 15 , // 16-19
            3  , 9  , 11 , 13 , // 20-23
            1  , 5  , 8  , 10 , // 24-27
            0  , 2  , 4  , 6  };// 28-31
        /**
         * VBAT Power-on Reset (PoR) flag
         * 
         * A variable storing whether the constructor detected a VBAT 
         * Power-on Reset (PoR) since the last shutdown. 
         */
        bool _vbat_por;
        /** A flag to signal whether SPI auto-sampling has been enabled
         *
         * This needs to be know because using the SPI for other purposes
         * while an auto-sampling transaction occurs can result in undefined
         * system behaviour. 
         */
        bool _is_autosampling;
        /** The name (string) of the current ADP transaction
         *
         * An ADP transaction (TX) can be used send data to the ADPDev python
         * scripts via ADP. This variable stores the name of the current 
         * transaction so it can properly terminal the transaction with the 
         * correct name when it has finished. 
         */
        const char* _adp_tx_name;
        /**
         * Set the inttimer (used by PCSM interrupt timer and autosampling)
         * 
         * The input is the number RTC tick periods (~30.03 us) to wait. 
         * Note that the value that is written to the PCSM to achieve this
         * is actually RTC ticks + 1, unless 0 is specified. 
         *
         * @param rtc_ticks The number of RTC tick periods. A value of 0 
         *     disables the counter. A value of 1 is not allowed. 
         */
        void _set_inttimer(uint32_t rtc_ticks);
        /**
         * A static system function for determining whether DEVE mode is on
         *
         * DEVE mode is read from status register 7 and needs to be checked 
         * before printing over ADP (sending standard out when DEVE mode is 
         * off will result in a lock-up). 
         */
        static bool _is_deve(void);
        /**
         * The control register RegClass instance
         *
         * Enables use of the control register (publicly accessible through 
         * the ctrl pointer variable. 
         */
        RegClass _ctrl;
        /**
         * The status register RegClass instance
         *
         * Enables use of the status register (publicly accessible through 
         * the status pointer variable. 
         */
        RegClass _status;
        /**
         * The AES block AESClass instance
         *
         * Enables use of the AES module (publicly accessible through 
         * the aes pointer variable. 
         */
        AESClass _aes;
        /**
         * The SPI block SPIClass instance
         *
         * Enables use of the SPI module (publicly accessible through 
         * the spi pointer variable. 
         */
        SPIClass _spi;
        /**
         * The GPIO block GPIOClass instance
         *
         * Enables use of the GPIO module (publicly accessible through 
         * the gpio pointer variable. 
         */
        GPIOClass _gpio;
        /**
         * The Shutdown RAM (SHRAM) block RegClass instance
         *
         * Enables use of the Shutdown RAM (publicly accessible through 
         * the shram pointer variable. 
         */
        RegClass _shram;
        /**
         * Sets the RTC Wakeup Register (used for timed-shutdown)
         *
         * If a zero is written to the RTC Wakeup Register (located in the 
         * PCSM), then the system goes into (deep) shutdown mode when a WFI 
         * occurs and the CPU deep sleep flag is set. If the RTC Wakeup 
         * register is set to any other value, then the system will in instead
         * enter a timed-shutdown and the value is the number of RTC ticks to 
         * wait for. 
         *
         * @param rtc_ticks The number of RTC ticks to enter a timed-shutdown
         *     for. If a zero is written, then a deep shutdown is instead 
         *     entered. 
         */
        void _set_rtc_wakeup(uint64_t rtc_ticks);
        /**
         * Clears the RTC wakeup register
         *
         * Equivalent to passing a zero to the _set_rtc_wakeup function.
         */
        void _clear_rtc_wakeup(void);
        /** Sets up and enables the SysTick Timer
         *
         * @param ticks The number of TCRO ticks to count
         */
        void _enable_systick(uint32_t ticks);
        /** Sets the perf with the raw HW ID
         *
         * @param perf the perfance level to set (raw HW ID value)
         */
        void _set_raw_perf(uint8_t perf);
        /** Returns the perf (the raw HW ID value)
         *
         * @return The current performance level (raw HW ID value)
         */
        uint8_t _get_raw_perf(void);
    public:
#ifdef M0N0_HEAP
#else
        /** Stores M0N0 system object when using static allocation
         *
         * I.e. only used if M0N0_HEAP flag is not set
         */
        static M0N0_System &sys_instance();
        /** Constructor when using static allocation
         *
         * I.e. only used if M0N0_HEAP flag is not set
         */
        M0N0_System(const M0N0_System &oth);
#endif
        /** Stores interrupt extwake hander callback function
         */
        Handler_Func _handler_extwake; // to make static?
        /** Stores systick interrupt hander callback function
         */
        Handler_Func _handler_systick; // to make static?
        /** Stores pcsm inttimer interrupt hander callback function
         */
        Handler_Func _handler_pcsm_inttimer; // to make static?
        /** Stores autosample interrupt hander callback function
         */
        Handler_Func _handler_autosample; // to make static?
        /** Flag for signalling autosampling should be disabled
         */
        bool autosample_disable_flag;
        /**
         * Number of RTC ticks in one millisecond
         *
         * Used for converting times periods in milliseconds to RTC ticks
         */
        static const uint64_t kRtcOneMsTicks = 33;
        /**
         * Time period of one RTC tick in microseconds
         *
         * Used for (inefficient but convenient) conversion from RTC ticks to
         * microseconds
         *  
         */
        static constexpr float kRtcPeriodUs = 30.3030303f;
        /**
         * Function that returns M0N0_System singleton instance
         *
         * Used (but different implementation) in both heap allocation and 
         * static allocation of M0N0 system. 
         * (heap allocation) The first time this function is called, it instantiates the 
         * M0N0_System (singleton) instance using the private constructor.
         * On subsequent calls, it returns the same instance. 
         */
        static M0N0_System* get_sys(void) {
#ifdef M0N0_HEAP
            if (_instance == NULL) {
                _instance = new M0N0_System();
            }
            return _instance;
#else
            return &M0N0_System::sys_instance();
#endif
        }
        /**
         * Function that returns M0N0_System singleton instance
         *
         * The first time this function is called, it instantiates the 
         * M0N0_System (singleton) instance using the private constructor.
         * On subsequent calls, it returns the same instance. 
         *
         * @param log_level Sets the level (debug, info etc.) of log messages
         *     to forward to standout out. This allows a global switch for 
         *     determining how much information is sent via STDOUT. 
         */
        static M0N0_System* get_sys(LOG_LEVEL_t log_level) {
#ifdef M0N0_HEAP
            if (_instance == NULL) {
                _instance = new M0N0_System();
            } else {
                _instance->set_log_level(log_level);
            }
            return _instance;
#else
            M0N0_System* temp = &M0N0_System::sys_instance();
            temp->set_log_level(log_level);
            return temp;
#endif
        }
        /**
         * A "printf" function for debug messages
         *
         * Whether the message is sent over STDOUT depends on the set log 
         * level and whether DEVE mode is enabled. The message is prepended 
         * with "DEBUG: " and a carriage return is added automatically. 
         *
         * @note Usage example: sys->log_debug("My message, val: %d", val);
         */
        int log_debug(const char *fmt, ...);
        /**
         * A "printf" function for informative messages
         *
         * Whether the message is sent over STDOUT depends on the set log 
         * level and whether DEVE mode is enabled. The message is prepended 
         * with "INFO: " and a carriage return is added automatically. 
         *
         * @note Usage example: sys->log_info("My message, val: %d", val);
         */
        int log_info(const char *fmt, ...);
        /**
         * A "printf" function for warning messages
         *
         * Whether the message is sent over STDOUT depends on the set log 
         * level and whether DEVE mode is enabled. The message is prepended 
         * with "WARN: " and a carriage return is added automatically. 
         *
         * @note Usage example: sys->log_warn("My message, val: %d", val);
         */
        int log_warn(const char *fmt, ...);
        /**
         * A "printf" function for error messages
         *
         * Whether the message is sent over STDOUT depends on whether DEVE
         *  mode is enabled. The message is prepended 
         * with "ERROR: " and a carriage return is added automatically. 
         *
         * @note This is intended for critical errors and printing cannot
         *     be suppressed. 
         *
         * @note Usage example: sys->log_error("My message, val: %d", val);
         */
        int log_error(const char *fmt, ...);
        /** 
         * A "printf" function
         *
         * Prints to STDOUT (which can be read via ADP). It checks whether 
         * DEVE mode is enabled and, if so, uses the printf functionality
         * defined in  m0n0_defs.c and send the message via ADP. 
         *
         * @note If DEVE is on, printf must be read from a computer connected
         *     via ADP, otherwise the internal buffer will fill and the
         *     system will halt. 
         */ 
        static int print(const char *fmt, ...);
        /**
         * Static function to print error and raise exception
         */
        static void error(const char *message);
        /**
         * Static function to print debug message
         */
        static void debug(const char *message);
        /**
         * pointer to instance of RegClass for accessing control registers
         */
        RegClass* ctrl;
        /**
         * pointer to instance of RegClass for accessing status registers
         */
        RegClass* status;
        /**
         * pointer to instance of AESClass for controlling the AES
         */
        AESClass* aes;
        /**
         * pointer to instance of SPIClass for controlling the SPI
         */
        SPIClass* spi;
        /**
         * pointer to instance of GPIOClass for controlling the GPIO
         */
        GPIOClass* gpio;
        /**
         * pointer to instance of RegClass for read/writing to SHRAM
         */
        RegClass* shram;
        /** Function to set the perf level
         *
         * Note that the perf does not update immediately after exiting
         * the function. 
         *
         * @param perf The perf level to set (0-15). 
         */
        void set_perf(uint8_t perf);
        /** Returns the current perf level
         * 
         * Note that the perf value does not update immediately and so 
         * reading the perf with get_perf after setting with set_perf
         * may return the old value. 
         */
        uint8_t get_perf(void);
        /** Reads the current RTC counter value
         *
         * @return the real-time clock counter value
         */
        uint64_t get_rtc(void); // get the raw 44-bit RTC value
        /** Reads the current RTC in microseconds (expensive)
         *
         * A convenient (but less efficient) for reading the RTC counter
         * in microseconds (us). 
         *
         * @return The RTC value in microseconds
         */
        float get_rtc_us(void); // get the microseconds elapsed. 
        /** Reads flag indicating whether RTC counter has stopped
         *
         * In (deep) shutdown mode the RTC stops. This flag indicates whether
         * the system has had a deep shutdown since the first VBAT turn-on
         *
         * @return whether the RTC counter has stopped (true) or not (false)
         */
        bool is_rtc_real_time(void); // read the real_time_flag of STATUS 7
        /** An active sleep (wait) for a specific number of RTC ticks
         *
         * A simple sleep function using the RTC. The system stays active,
         * blocking for the specific number of RTC ticks.
         *
         * @param rtc_ticks the number of RTC ticks to wait for
         */
        void sleep_rtc(uint64_t rtc_ticks);
        /** An active sleep (wait) for a specific number of milliseconds
         *
         * A simple sleep function using the RTC. The system stays active,
         * blocking for the specific number of milliseconds
         *
         * @param time_ms the number of milliseconds to sleep for 
         * (timed using the RTC)
         */
        void sleep_ms(uint32_t time_ms);
        /** Runs a testcase (workload)
         *
         * Testcases defined in tc_functions.h can be run using their ID. 
         * They return a pass/fail flag. 
         *
         * @param tc The ID of the defined testcase to run
         * @param verbose A "1" indicates that printf output should be 
         *     preserved, while a "0" indicates that it should be suppressed
         * @param repeat_delay "0" means that the testcase (workload) should 
         *     be run once, while a value greater than 0 means that the 
         *     testcase should be repeated for that number or RTC ticks. 
         *     During the repeats, all standard output is suppressed (to 
         *     avoid) high frequency standard output from blocking the ADP).
         *     The testcase is then run once with standard output after the 
         *     repeats. The repeat_delay is intended for power measurement 
         *     purposes. 
         * @return A 0 (pass) or a 1 (fail) depending on whether the testcase
         *     passed or failed (see tc_functions.h) for the return code 
         *     definitions.
         */
        int run_testcase(testcase_id_t tc, uint32_t verbose, uint64_t repeat_delay);
        /** The software goes into a loop waiting for ADP direction via CTRL5
         *
         * The software enters a loop where it is polling for a command issued
         * by the ADPDev python scripts (run_testcase) via Control Register 5.
         * The run_testcase function updates CTRL 5 with the testcase ID and 
         * repeat delay. The embedded software then runs its run_testcase 
         * function with those parameters. 
         *
         * @param timeout_ms The time to stay in the wait for ADP loop before 
         *     exiting to continue execution. If it is zero, then the loop
         *     is infinite, with no timeout
         * @param verbose Whether the verbose flag is sent to testcases
         */
        void wait_for_adp(
                uint32_t timeout_ms, // zero means no timeout - forever
                uint32_t verbose);
        /** Powers off the ROM banks to save energy
         */
        void power_off_roms(void);
        /** Sets the Cortex-M33 Deep Sleep flag
         *
         * If a wait for interrupt (WFI) occur with the deep sleep flag
         * set, then the system goes into a shutdown mode. Whether the
         * shutdown mode is timed or non-timed (deep), depends on the 
         * value in the RTC wakeup PCSM register. 
         */
        void set_cpu_deepsleep(void);
        /** Clears the Cortex-M33 Deep Sleep flag
         *
         * If a wait for interrupt (WFI) occur without the deep sleep flag
         * set, then the system does not go into a shutdown mode. 
         */
        void clear_cpu_deepsleep(void);
        /**
         * Enter a system timed-shutdown for a specific number of RTC ticks
         *
         * After entering a timed shutdown mode, the system will wake up 
         * (software will execute from the beginning) after the specified 
         * number of RTC ticks. 
         * The EXTWAKE signal cannot wake up the system in timed-shutdown 
         * mode.
         * Unlike (deep) shutdown mode, the RTC counter continues to count.
         *
         * @param rtc_ticks The number of RTC ticks to wait before waking up 
         *     (note that 0 is not a valid value). 
         */
        void timed_shutdown(uint64_t rtc_ticks);
        /**
         * Enter a system timed-shutdown for a specific number of milliseconds
         *
         * After entering a timed shutdown mode, the system will wake up 
         * (software will execute from the beginning) after the specified 
         * number of milliseconds (timed with the RTC). 
         * The EXTWAKE signal cannot wake up the system in timed-shutdown 
         * mode.
         * Unlike (deep) shutdown mode, the RTC counter continues to count.
         *
         * @param time_ms The number of milliseconds to wait before waking up
         *     (note that 0 is not a valid value). 
         */
        void timed_shutdown_ms(const uint32_t time_ms);
        /**
         * Enter a system (deep) shutdown mode.
         *
         * After entering a deep shutdown mode, the system can only be woken
         * up by a signal on the EXTWAKE input pin. 
         * Unlike the shutdown mode, the RTC counter stops counting. 
         *
         */
        void deep_shutdown(void);
        /**
         * Reads next character from the Standard Input (STDIN) from ADP
         *
         * This function waits (blocks) until the next character is available.
         *
         * @return A received character
         */
        char wait_read_stdin(void);
        /**
         * Sets safe, but more optimal, system register values. 
         *
         * Some system register values can be changed from their 
         * Power-on-Reset (PoR) values to more optimal settings that
         * have been tested to be safe in silicon testing. This function
         * writes these more optimal system settings and is intended to be
         * called early in the software startup, just after the instantiation
         * of the M0N0 system. Note that the ROM power-on delay PoR value
         * is changed in the system constructor. 
         *
         */
        void set_recommended_settings(void);
        /**
         * Determines whether VBAT has been reset or not
         *
         * After the software starts, this function can be called to 
         * determine whether the VBAT domain has been reset or not (i.e. 
         * is the system waking up from a shutdown or a cold start). If there
         * has not been a VBAT reset, it can be assumed that the PCSM
         * register values have persisted and Shutdown RAM is intact. 
         *
         * @return A flag determining whether there has been a VBAT reset 
         *     (true) or not (false). 
         */
        bool is_vbat_por(void);
        /** Determines whether the EXTWAKE signal is currently exerted
         *
         * Also see the enable_extwake_interrupt method. 
         *
         * @return True if EXTWAKE is currently exerted, false otherwise
         */
        bool is_extwake(void);
        /** Utility method for printing basic system information
         *
         * Prints current system status information via Standard Output
         * (STDOUT). 
         */
        void print_info(void);
        /** Generates a hardfault for testing the hardfault handler
         *
         * For testing purposes only. 
         */
        void generate_hardfault(void);
        /** Enables an interrupt on EXTWAKE exertion with a custom handler
         * 
         * The extwake interrupt is enabled and the extwake interrupt
         * handler defined (by default) in the interrupts.c file is 
         * called (Interrupt6_Handler). This file is shared by 
         * multiple projects (by default)
         * and it is designed to call a function specified by this function.
         * 
         * @param f A pointer to the callback function to call from the 
         *     interrupt handler. Must match the Handler_Func definition
         *     (void return type and void parameters). If no callback 
         *     function is required, then NULL should be passed. 
         */
        void enable_extwake_interrupt(Handler_Func f);
        /** Disables the extwake interrupt and removes the hander
        */
        void disable_extwake_interrupt(void);
        /** Enables the Cortex-M33 SysTick Timer. 
         *
         * @param ticks the number of TCRO ticks before the SysTick
         *     interrupt occurs, calling the SysTick interrupt handler
         *     (by default) defined in interrupts.c (SysTick_Handler). 
         * @param f A pointer to a callback function to call from the 
         *     interrupt hander. If no callback is to be set, NULL must
         *     be passed. 
         */
        void enable_systick(uint32_t ticks, Handler_Func f);
        /** Disables the SysTick Timer and corresponding interrupt
         */
        void disable_systick(void);
        /**
         * Enable PCSM SPI autosampling
         *
         * Note that this is difficult to safely turn off (see 
         * disable_autosampling) 
         */
        void enable_autosampling_rtc_ticks(uint32_t rtc_ticks, Handler_Func f);
        /**
         * Enables SPI autosampling mechanism with the desired sample period
         * and callback function
         * 
         * @param rtc_ticks The RTC ticks between each sample. While the 
         *     hardware counts to the specified number of ticks plus one, 
         *     this function subtracts one to compensate. 
         *     0 and 1 is not valid. 
         * @param f A pointer to a callback function to call after four bytes
         *     have been sampled 
         * @note While autosampling is enabled, the SPI must not be used for
         *     any other purpose (including writing to the PCSM) and it must
         *     be switched off before entering any shutdown mode (which is 
         *     handled by the dedicated shutdown functions). 
         */
        void enable_autosampling_ms(uint32_t interval_ms, Handler_Func f);
        /** 
         * Disable PCSM SPI autosampling (blocking)
         *
         */
        void disable_autosampling_wait(void);
        /** 
         * Disable PCSM SPI autosampling (no wait)
         *
         * Autosampling must be disabled carefully. If there is an SPI 
         * transaction at the same time as an auto sample, the SPI can end 
         * up in an undefined state, which can only be fixed through a 
         * VBAT reset. This function must be called directly after last
         * autosampling interrupt before next SPI transaction initiates
         *
         */
        void disable_autosampling(void);
        /** 
         * Send ADP transaction start header
         *
         * Sends an ADP transaction start header that is recognised by the 
         * ADPDev scripts. Contains a string name of the transaction
         *
         * @param name String that is sent to the ADPDev to identify the type
         *     of transaction.  
         */
        void adp_tx_start(const char *name);
        /** 
         * Sent after the (option) parameters have been sent
         *
         * If parameters are sent in the ADP transaction, this command
         * splits the parameters from the payload of the transaction. 
         * If there are no parameters, then this does not have to be sent. 
         */
        void adp_tx_end_of_params();
        /** 
         * Send end transaction message to ADPDev
         *
         */
        void adp_tx_end();
        /** 
         * Sets the level of log messages that are sent via STDOUT
         *
         * The messages sent via STDOUT (using log_info, or log_error) 
         * can be filtered. This function sets the minimum level of message
         * to be sent via STDOUT over ADP
         *
         * @param log_level The minimum message level to send
         */
        void set_log_level(LOG_LEVEL_t log_level);
        /**
         * Turns on the PCSM interrupt time (loop timer) to raise an interrupt
         * after the specified time. 
         *
         * @param The time interval after which to raise an interrupt
         *        (in milliseconds). 
         * @param f A pointer to a callback function to execute when the 
                    interrupt occurs (pass NULL if to not execute a callback
                    function)
         */
        void enable_pcsm_interrupt_timer_ms(uint32_t interval_ms, Handler_Func f);
        /**
         * Turns on the PCSM interrupt time (loop timer) to raise an interrupt
         * after the specified time. Interrupt occurs periodically until
         * disabled.
         *
         * @param The time interval after which to raise an interrupt
         *        (in RTC clock cycles). 
         * @param f A pointer to a callback function to execute when the 
                    interrupt occurs (pass NULL if to not execute a callback
                    function)
         */
        void enable_pcsm_interrupt_timer_rtc_ticks(uint32_t rtc_ticks, Handler_Func f);
        /**
         * Disables the PCSM interrupt timer (loop timer).
         * after the specified time. Interrupt occurs periodically until
         * disabled.
         *
         * @param The time interval after which to raise an interrupt
         *        (in RTC clock cycles). 
         * @param f A pointer to a callback function to execute when the 
                    interrupt occurs (pass NULL if to not execute a callback
                    function)
         */
        void disable_pcsm_interrupt_timer(void);
        /**
         * A utility function for estimating the current TCRO frequency using
         * by comparing it to the RTC clock
         *
         * @return the estimated TCRO frequency in kHz
         */
        uint32_t estimate_tcro(void);
};

#endif // M0N0_H
