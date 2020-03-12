
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
#ifndef SYSUTIL_H
#define SYSUTIL_H
#include <cstddef>

#include "tc_functions.h" // required for GPIO protocol enums

extern "C" {
    #include "m0n0_defs.h"
}

typedef uint32_t (*Read_Driver_Func)(uint32_t);
typedef void (*Write_Driver_Func)(uint32_t, uint32_t);
typedef uint32_t (*Read_BG_Driver_Func)(uint32_t, uint32_t);
typedef void (*Write_BG_Driver_Func)(uint32_t, uint32_t, uint32_t);
typedef void (*Log_Func)(const char*);

static const MEM_RD_WR_t REG_MEM_READ = R;
static const MEM_RD_WR_t REG_MEM_WRITE = W;
static const MEM_RD_WR_t REG_MEM_READ_WRITE = RW;

/**
 * Generic class for accessing registers/memory
 * 
 * A class for providing a standard interface for  the reading and/or writing
 * to system registers and memories. Read and write "drivers" are passed as
 * function pointers. Performs extra checking (i.e. whether provided address
 * is in the register/memory address range) if EXTRA_CHECKS is defined. 
 * It also forms the base class for other classes, such as AESClass and 
 * SPIClass. 
 */
class RegClass {
    private:
        /** The base address of the register or memory
         */
        uint32_t _base;
        /** The size of the register or memory
         */
        uint32_t _size;
        /** Whether the register is read only, write only or read/write
         */
        MEM_RD_WR_t _read_write;
        /** Whether to add the base address to each address passed to 
         * this object or not. 
         *
         * Whether address passed to this object (i.e. through read/write
         * functions) are the absolute address (already include the offset)
         * or the relative address (without offset included). For example,
         * within the M0N0 System all registers (including AES and SPI) 
         * have the absolute address passed into the functions (and this
         * variable is false as the offset is included), whereas it is more 
         * convenient to pass the relative address for memories (e.g.
         * Shutdown RAM) and therefore this variable should be true so that
         * the base address is automatically added. 
         * 
         */
        bool _add_offset;
        /** A pointer to the function used to read the register/memory
         * (NULL if not applicable)
         */
        Read_Driver_Func _read_driver_f;
        /** A pointer to the function used to write the register/memory
         * (NULL if not applicable)
         */
        Write_Driver_Func _write_driver_f;
        /** A pointer to the function used to read a bit grou of a 
         *  register/memory (NULL if not applicable)
         */
        Read_BG_Driver_Func _read_bg_driver_f;
        /** A pointer to the function used to write a bit grou of a 
         *  register/memory (NULL if not applicable)
         */
        Write_BG_Driver_Func _write_bg_driver_f;
        /**
         * A function for checking the passed address is in the valid address
         * range.
         */
        bool _addr_check(uint32_t address);
    public:
        /** 
         * Constructor for RegClass
         *
         * @param base_address Base address of the register/memory in the 
         *     memory map
         * @param add_offset whether the addresses passed to the read/write 
         *     functions is the absolute address (add_offset=False) or 
         *     whether it needs to be added to the base_address 
         *     (add_offset=True). Typically, for a register
         *     this value is False (and the absolute address passed) and 
         *     for memories it is True. 
         * @param size The size of the register or memory (in bytes)
         * @param read_or_write Specify whether the register or memory is
         *     read-only write-only, or read-and-write, using the 
         *     MEM_RD_WR_t enum. 
         * @param read_driver A pointer to a function for reading a whole 
         *     register 
         *     or memory location. 
         * @param write_driver A pointer to a function for writing a whole
         *     register or memory location.
         * @param read_bg_driver A pointer to a function for reading from a
         *     specific bit-group (or "bit range") of a register using a 
         *     supplied mask. 
         * @param write_bg_driver A pointer to a function for writing to a 
         *     specific bit-group of a register using a specified mask.  
         * @param error_function A pointer to a function for logging an 
         *     logging an error message and halting execution
         * @param A pointer to a function for displaying debug text output
         */
        RegClass(
                uint32_t base_address,
                bool add_offset,
                uint32_t size,
                MEM_RD_WR_t read_or_write,
                Read_Driver_Func read_driver,
                Write_Driver_Func write_driver,
                Read_BG_Driver_Func read_bg_driver,
                Write_BG_Driver_Func write_bg_driver,
                Log_Func error_function,
                Log_Func debug_function
        );
        /**
         * A function for reading a whole register or memory address.
         *
         * @param address The register or memory address. Whether this is the
         *     the absolute address or an offset depends on the value of 
         *     add_offset passed to the constructor.  
         * @return the data read from the specified location. 
         */
        uint32_t read(uint32_t address);
        /**
         * A function for reading a bit group of a register or memory address.
         *
         * @param address The register or memory address. Whether this is the
         *     the absolute address or an offset depends on the value of 
         *     add_offset passed to the constructor.  
         * @param mask The bit-group mask to read from. 
         * @return the data read from the specified bit group of the location.
         */
        uint32_t read(uint32_t address, uint32_t mask);
        /**
         * A function for write to a  register or memory address.
         *
         * Masking and shifting is applied automatically. 
         *
         * @param address The register or memory address. Whether this is the
         *     the absolute address or an offset depends on the value of 
         *     add_offset passed to the constructor.  
         * @param data The data to be written to the address
         */
        virtual void write(uint32_t address, uint32_t data);
        /**
         * A function for write to a bit-group of a register or memory address.
         * 
         * Masking and shifting is applied automatically. 
         *
         * @param address The register or memory address. Whether this is the
         *     the absolute address or an offset depends on the value of 
         *     add_offset passed to the constructor.  
         * @param mask The bit-group mask
         * @param data The data to be written to the address
         */
        virtual void write(
                uint32_t address,
                uint32_t mask,
                uint32_t data);
        Log_Func _debug_f;
        Log_Func _error_f;
};

class SPIClass : public RegClass {
    using RegClass::RegClass;
    private:
        /** PCSM slave select
         */
        static const SPI_SS_t kPcsmSS; // PCSM Slave select
        /** Software flag to remember whether auto-sampling enabled
         */
        bool _is_autosampling = false;
    public:
        /**
         * Sets the SPI clock divider (divides the TCRO frequency). 
         *
         * @param div The divider: resulting frequency is 
         *      1 / (2*(1+div))
         */
        void set_clk_divide(uint32_t div);
        /** Returns the current clock divider
         */
        uint32_t get_clk_divide() ;
        /** Sets the SPI clock polarity and phase mode
         * 
         * @param mode The SPI mode (valid values are 0-3)
         *     (see the M0N0 TRM for the polarity and phase lookup table)
         */
        void set_mode(uint8_t mode) ;
        /** Reads the current SPI mode
         *
         * @return The SPI phase and polarity mode (0-3)
         */
        uint32_t get_mode();
        /** Sets the SPI slave select
         *
         * @param slave_id The slave select enum (SPI_SS_t is defined in 
         *     m0n0_defs.h)
         */
        void set_slave(SPI_SS_t slave_id);
        /** Reads the current slave select ID
         *
         * @return The current slave select ID (SPI_SS_t is defined in
         *     m0n0_defs.h
         */
        SPI_SS_t get_slave();
        /** Writes a byte via SPI to the specified slave
         *
         * @param slave_id The slave to send the byte to. The specified slave
         *     remains set after the write. 
         * @param data The byte to send over SPI
         */
        uint8_t write_byte(SPI_SS_t slave_id, uint8_t data);
        /** Writes a byte via SPI to the currently selected slave
         *
         * @param data The byte to send over SPI
         *
         */
        uint8_t write_byte(uint8_t data);
        /**
         * Writes data to a specified PCSM register via SPI
         *
         * @param address The PCSM register address
         * @param data The data to write to the PCSM register 
         *     (only the 24 LSBs are written)
         */
        void pcsm_write(uint8_t address, uint32_t data);
        /** Writes to the VREG SPI registers, see the M0N0Reg write functions
         */
        void write(uint32_t address, uint32_t data) override;
        /** Writes to the VREG SPI registers, see the M0N0Reg write functions
         */
        void write(
                uint32_t address,
                uint32_t mask,
                uint32_t data) override;
        /**
         * Is autosampling feature enabled
         *
         * @return the flag indicating if auto-sampling is enabled
         */
        bool get_is_autosampling(void);
        /**
         * Enables SPI auto-sampling
         *
         */
        void enable_autosampling(void);
        /**
         * Disables SPI auto-sampling
         *
         */
        void disable_autosampling(void);
        
};

class GPIOClass : public RegClass {
    public:
        GPIOClass(
                uint32_t base_address,
                bool add_offset,
                uint32_t size,
                MEM_RD_WR_t read_or_write,
                Read_Driver_Func read_driver,
                Write_Driver_Func write_driver,
                Read_BG_Driver_Func read_bg_driver,
                Write_BG_Driver_Func write_bg_driver,
                Log_Func error_function,
                Log_Func debug_function
            ): RegClass(
                    base_address,
                    add_offset,
                    size,
                    read_or_write,
                    read_driver,
                    write_driver,
                    read_bg_driver,
                    write_bg_driver,
                    error_function,
                    debug_function
            ) {
            this->_gpio_protocol = false; 
        }
        /**
         * Sets the four GPIO pins
         */
        void write_data(uint8_t data); 
        /**
         * Reads the status of the four GPIO pins
         */
        uint8_t read_data(); 
        /**
         * Sets whether each pin is an input or output
         */
        void set_direction(uint8_t direction); 
        /**
         * Reads whether each pin is an input or output
         */
        uint8_t get_direction(); 
        /**
         * (Not yet tested)
         */
        void set_interrupt_mask(uint8_t mask);
        /** GPIO protocol is a utility targeted at simulation testing
         * (not expected to be used with a chip)
         */
        void enable_gpio_protocol(); 
        /** GPIO protocol is a utility targeted at simulation testing
         * (not expected to be used with a chip)
         */
        void disable_gpio_protocol(); 
        /** GPIO protocol is a utility targeted at simulation testing
         * (not expected to be used with a chip)
         */
        void protocol_tc_start(testcase_id_t tc_id);
        /** GPIO protocol is a utility targeted at simulation testing
         * (not expected to be used with a chip)
         */
        void protocol_tc_end(testcase_id_t tc_id);
        /** GPIO protocol is a utility targeted at simulation testing
         * (not expected to be used with a chip)
         */
        void protocol_event(gpio_evt_id_t evt_id);
    private:
        bool _gpio_protocol;
        void _protocol_send_raw(gpio_sig_id_t id, uint8_t payload);
};

class RTCTimer {
    private:
        uint64_t _start_ticks;
        uint64_t _interval;
    public:
        RTCTimer();
        /** Resets the internal _start_ticks
         */
        void reset(); // resets the start_ticks to current
        /** Returns cycles elapsed since the _start_ticks
         *
         * @return RTC cycles since last reset
         */
        uint64_t get_cycles(); // sets the RTC cycles since reset()
        /** Returns microseconds elapsed since the _start_ticks
         *
         * Note that this function is inefficient and provided solely for 
         * convenience. 
         *
         * @return Microseconds elapsed since the last reset
         *
         */
        float get_us(); // same as get_cycles() but converts to microseconds
        /** Sets the software timer interval
         * 
         * @param interval_cycles The number of RTC cycle periods that make
         *     up the time period
         */
        void set_interval(uint64_t interval_cycles); // setup interval
        /** Sets the software timer interval in milliseconds
         * 
         * @param interval_ms The time interval specified in milliseconds
         */
        void set_interval_ms(uint32_t interval_ms); // setup interval (in ms)
        /**
         * Checks whether the interval has elapsed. 
         *
         * @return Returns true if the interval has elapsed, false otherwise
         */
        bool check_interval(); // returns true if time has elapsed 
        /**
         * Waits in a loop for the specified time period
         */
        void wait();
        /**
         * Lowers the current DVFS level to the minimum value and waits
         * in a loop for the specified time period, before restoring
         * the DVFS level on exit
         */
        void wait_lp();
        /**
         * In addition to reducing the DVFS level, it Uses the PCSM interrupt
         *  timer ("loop timer") to wait for the specified time
         */
        void wait_lp_inttimer();
};

class AESClass : public RegClass {
    using RegClass::RegClass;
    public:
        void set_key(uint32_t key[8]);
        uint32_t* get_key(uint32_t res[8]);
        /**
         * Encrypt data (and wait for result)
         *
         * @param data pointer to data array
         * @param data_size length of the data array
         * @param result pointer to array in which to store the result
         * @note array length must be a multiple of 4
         */
        void encrypt_blocking(
                uint32_t* data,
                const uint32_t data_size,
                uint32_t* result);
        /**
         * Decrypt data (and wait for result)
         *
         * @param data pointer to data array
         * @param data_size length of the data array
         * @param result pointer to array in which to store the result
         * @note array length must be a multiple of 4
         */
        void decrypt_blocking(
                uint32_t* data,
                const uint32_t data_size,
                uint32_t* result);
        void encrypt_irq(const uint32_t* data, const uint32_t data_size);
        void decrypt_irq(const uint32_t* data, const uint32_t data_size);
    private:
        void _write_data(const uint32_t* data);
        void _read_data(uint32_t* res);
        void _wait_for_completion();
        void _en_encryption();
        void _en_decryption();
        void _start();
        void _enable_irq();
        void _disable_irq();
        void _clear_irq();
        void _reset_clear_irq();
};


// A circular buffer that can be stored in SHRAM
class CircBuffer {
    private:
        uint16_t _head; // only 15 bits saved and restored
        uint16_t _tail; // only 15 bits saved and restored
        uint16_t _size;
        uint32_t* _buffer;
        bool _full;
        uint32_t _shram_address;
        // if _allow_overwrite is set, the buffer will always append, 
        // overwriting old data is full
        bool _allow_overwrite;
        Handler_Func _full_error_callback;
        Handler_Func _empty_error_callback;
        Handler_Func _read_error_callback;
        uint32_t _total_appends;
        uint32_t _total_removes;
    public:
        CircBuffer(
                uint32_t* array = NULL,
                uint32_t size = 0,
                uint32_t shram_address = 0,
                bool allow_overwrite = false,
                Handler_Func full_error_func = NULL,
                Handler_Func empty_error_func = NULL,
                Handler_Func read_error_func = NULL);
        void reset(void);
        bool is_empty(void);
        bool is_full(void);
        uint32_t get_capacity(void);
        uint32_t get_length(void);
        bool append(uint32_t item);
        bool remove(uint32_t* data);

        uint32_t get_total_appends(void);
//        uint32_t get_total_removes(void);
        bool read(uint32_t position, uint32_t* data);

        void store_to_shram();
        void load_from_shram();

        /**
         * Converts buffer to an array
         * 
         * Creates an array with the first element being the first element 
         * appended, and the last element is the last element appended. 
         * Only contains present elements in the array and the length is 
         * therefore the number of occupied elements. 
         *
         * @note This function is relatively expensive. 
         */
        void to_array(uint32_t* array);

        /** Prints the full buffer to STDOUT
         *
         * Verbose printing of full buffer, including tail and head positions 
         * and flags.
         */
        void print(void);
        /** Prints the raw buffer
         *
         * Prints the raw buffer, including empty elements. The size is the
         * length of the buffer. 
         */
        void print_raw_buffer(void);
        /** Prints in a format suited for ADP
         */
        void send_via_adp(void);

        /** Prints the array of present values
         *
         * Prints the array of present values (length of the array is the
         * the number of inserted items, not the length of the buffer). 
         * Uses the to_array function.
         *
         */
        void print_array(void);
        /**
         * Returns the sample number for an element
         * 
         * Gets the sample number of the item at a particular 
         * index in the buffer (uses the total_appends count)
         */
        uint32_t get_sample_count(uint32_t position);
};


#endif // SYSUTIL_H

