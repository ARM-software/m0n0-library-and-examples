
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
#include "sysutil.h"
#include "m0n0.h"
#include <cstdarg>

extern "C" {
    #include "m0n0_defs.h"
    #include "m0n0_printf.h"
}

RegClass::RegClass(
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
        ) {
    this->_base = base_address;
    this->_add_offset = add_offset;
    this->_size = size;
    this->_read_write = read_or_write;
    this->_read_driver_f = read_driver;
    this->_write_driver_f = write_driver;
    this->_read_bg_driver_f = read_bg_driver;
    this->_write_bg_driver_f = write_bg_driver;
    this->_error_f = error_function;
    this->_debug_f = debug_function;
}

// checks that the address is within range
bool RegClass::_addr_check(uint32_t address) {
    if (address >= this->_base && address <= (this->_base + this->_size)) {
        return 1;
    } else {
        this->_error_f("Address is out of register range");
        return 0;
    }
}

uint32_t RegClass::read(uint32_t address) {
    if (this->_add_offset) {
        address += this->_base;
    }
#ifdef EXTRA_CHECKS
    if (this->_read_driver_f == NULL) {
        this->_error_f("No read driver - is this register write only?");
    }
    this->_addr_check(address);
#endif
    return this->_read_driver_f(address);
}

uint32_t RegClass::read(
        uint32_t address,
        uint32_t mask) {
    if (this->_add_offset) {
        address += this->_base;
    }
#ifdef EXTRA_CHECKS
    if (this->_read_bg_driver_f == NULL) {
        this->_error_f("No read_bg driver - is this register write only?");
    }
    this->_addr_check(address);
#endif
    return this->_read_bg_driver_f(address,mask);
}

void RegClass::write(uint32_t address, uint32_t data) {
    if (this->_add_offset) {
        address += this->_base;
    }
#ifdef EXTRA_CHECKS
    if (this->_write_driver_f == NULL) {
        this->_error_f("No write driver - is this register read only?");
    }
    this->_addr_check(address);
#endif
    return this->_write_driver_f(address,data);
}

void RegClass::write(
        uint32_t address,
        uint32_t mask,
        uint32_t data) {
    if (this->_add_offset) {
        address += this->_base;
    }
#ifdef EXTRA_CHECKS
    if (this->_write_bg_driver_f == NULL) {
        this->_error_f("No write_bg driver - is this register read only?");
    }
    this->_addr_check(address);
#endif
    return this->_write_bg_driver_f(address,mask,data);
}

// ---------- M0N0 AES    ---------- //

void AESClass::set_key(uint32_t key[8]) {
    this->write(AES_KEY_0_REG,key[0]);
    this->write(AES_KEY_1_REG,key[1]);
    this->write(AES_KEY_2_REG,key[2]);
    this->write(AES_KEY_3_REG,key[3]);
    this->write(AES_KEY_4_REG,key[4]);
    this->write(AES_KEY_5_REG,key[5]);
    this->write(AES_KEY_6_REG,key[6]);
    this->write(AES_KEY_7_REG,key[7]);
}

void AESClass::_wait_for_completion() {
    uint32_t debug_count = 0; 
    while (this->read(AES_STATUS_REG) != 1) {
        debug_count++;
    }
    m0n0_printf("Debug_count: %d\n",debug_count);
    this->_debug_f("Completed");
}

void AESClass::_write_data(const uint32_t* data) {
    // Send data over 4 AHB transactions
    this->write(AES_DATA_0_REG,*(data+0));
    this->write(AES_DATA_1_REG,*(data+1));
    this->write(AES_DATA_2_REG,*(data+2));
    this->write(AES_DATA_3_REG,*(data+3));
}

void AESClass::_read_data(uint32_t* res) {
    res[0] = this->read(AES_DATA_0_REG);
    res[1] = this->read(AES_DATA_1_REG);
    res[2] = this->read(AES_DATA_2_REG);
    res[3] = this->read(AES_DATA_3_REG);
}

void AESClass::_en_encryption() {
    this->write(AES_CONTROL_REG, AES_R12_ENCRYPT_OR_DECRYPT_BIT_MASK,0);
}

void AESClass::_en_decryption() {
    this->write(AES_CONTROL_REG, AES_R12_ENCRYPT_OR_DECRYPT_BIT_MASK,1);
}

void AESClass::encrypt_blocking(
        uint32_t* data,
        const uint32_t data_size,
        uint32_t* result) {
	this->_clear_irq();
    this->_en_encryption();
    for (uint32_t i = 0; i < data_size; i+=4) {
        uint32_t* frame = (data+i);
        this->_write_data(frame);
        this->_debug_f("Starting AES encryption...");
        this->_start();
        this->_wait_for_completion();
        uint32_t res[4] = {0,0,0,0};
        this->_read_data(res);
        result[i] = res[0];
        result[i+1] = res[1];
        result[i+2] = res[2];
        result[i+3] = res[3];
    }
}

void AESClass::decrypt_blocking(
        uint32_t* data,
        const uint32_t data_size,
        uint32_t* result) {
    this->_clear_irq();
    this->_en_decryption();
    for (uint32_t i = 0; i < data_size; i+=4) {
        uint32_t* frame = (data+i);
        this->_write_data(frame);
        this->_debug_f("Starting AES decryption...");
        this->_start();
        this->_wait_for_completion();
        uint32_t res[4] = {0,0,0,0};
        this->_read_data(res);
        result[i] = res[0];
        result[i+1] = res[1];
        result[i+2] = res[2];
        result[i+3] = res[3];
    }
}

void AESClass::_start() {
    this->write(AES_CONTROL_REG, AES_R12_START_BIT_MASK, 1);
}

void AESClass::_enable_irq() {
    this->write(AES_CONTROL_REG, AES_R12_IRQ_ENABLE_BIT_MASK, 1);
}

void AESClass::_disable_irq() {
    this->write(AES_CONTROL_REG, AES_R12_IRQ_ENABLE_BIT_MASK, 0);
}

// Interrupt must be cleared in the interrupt handler
void AESClass::_clear_irq() {
    this->write(AES_CONTROL_REG, AES_R12_IRQ_CLEAR_FLAG_BIT_MASK, 1);
}

void AESClass::_reset_clear_irq(){
    this->write(AES_CONTROL_REG, AES_R12_IRQ_CLEAR_FLAG_BIT_MASK, 0);
}

const SPI_SS_t SPIClass::kPcsmSS = SS3;

bool SPIClass::get_is_autosampling(void) {
    return this->_is_autosampling;  
}

void SPIClass::enable_autosampling() {
	this->write(
            SPI_CONTROL_REG,
            SPI_R05_ENABLE_AUTO_SAMPLE_BIT_MASK,
            1);
    SPI_SS_t temp = SS2; // SS2 is only slave select
    this->set_slave(temp);
    this->_is_autosampling = true; // after so warning doesn't go off
}

void SPIClass::disable_autosampling() {
    this->_is_autosampling = false; // before so warning doesn't go off
	this->write(
            SPI_CONTROL_REG,
            SPI_R05_ENABLE_AUTO_SAMPLE_BIT_MASK,
            0);
}

void SPIClass::set_clk_divide(uint32_t div) {
    return this->write(SPI_CLK_DIVIDE_REG, div);
}

uint32_t SPIClass::get_clk_divide() {
    return this->read(SPI_CLK_DIVIDE_REG);
}

void SPIClass::set_mode(uint8_t mode) {
	this->write(
            SPI_CONTROL_REG,
            SPI_R05_CLK_POLARITY_PHASE_BIT_MASK,
            mode);
}

uint32_t SPIClass::get_mode() {
	return this->read(
            SPI_CONTROL_REG,
            SPI_R05_CLK_POLARITY_PHASE_BIT_MASK);
}

void GPIOClass::write_data(uint8_t data) {
    this->write(GPIO_DATA_REG,data);
}

uint8_t GPIOClass::read_data() {
    return this->read(GPIO_DATA_REG);
}

void GPIOClass::set_direction(uint8_t direction) {
#ifdef EXTRA_CHECKS
    if (direction >= 16) { // 2^4
        this->_error_f("Value passed to GPIO set_direction too large");
    }
#endif
    this->write(GPIO_DIRECTION_REG,direction);
}

void GPIOClass::set_interrupt_mask(uint8_t mask) {
    this->write(GPIO_INTERRUPT_REG, mask);
}

uint8_t GPIOClass::get_direction() {
    return this->read(GPIO_DIRECTION_REG);
}

void GPIOClass::enable_gpio_protocol() {
    this->_debug_f("Enabling GPIO protocol");
    this->_gpio_protocol = true;
    this->write_data(0x0); // set to zero
    this->set_direction(0xF); // set all as output
    this->write_data(0x0); // set to zero
}

void GPIOClass::disable_gpio_protocol() {
    this->_debug_f("Disabling GPIO protocol");
    this->_gpio_protocol = false;
    this->write_data(0x0); // set to zero
    this->set_direction(0x0); // set all as output
}

void GPIOClass::_protocol_send_raw(gpio_sig_id_t id, uint8_t payload) {
    // 1. Set strobe to 0
    // 2. Create header - LSB is always 0 for header
    // id in the 2 'middle' bits and 0 in the LSB
    uint8_t header0  = id << 1 & ~(1<<3); //LSB=0
    this->write_data(header0);
    this->write_data(header0 | (1<<3)); //strobe
    // second 'header'
    uint8_t header1 = (((id >> 2) << 1) & ~(1<<3)) | 1; //LSB=1
    this->write_data(header1);
    this->write_data(header1 | (1<<3)); //strobe
    for (int i = 0; i < 4; i++) {
        uint8_t payload_data = ((((payload >> (2*i)) << 1) | 1) & ~(1<<3));
        this->write_data(payload_data);
        this->write_data(payload_data | (1<<3)); //strobe
    }
    // Transaction only decoded when next transaction starts, send
    // dummy start of new transaction
    // Send the same header again - why not?
    // (i.e. so that the receiver isn't [is less likely] to be
    // confused by other GPIO usage).
    this->write_data(header0); //LSB=0
    this->write_data(header0 | (1<<3)); //strobe
}

void GPIOClass::protocol_tc_start(testcase_id_t tc_id) {
    if (!this->_gpio_protocol) {
        return;
    }
    gpio_sig_id_t sig = START_TC;
    this->_protocol_send_raw(sig, (int)tc_id);
}

void GPIOClass::protocol_tc_end(testcase_id_t tc_id) {
    if (!this->_gpio_protocol) {
        return;
    }
    gpio_sig_id_t sig = END_TC;
    this->_protocol_send_raw(sig, (int)tc_id);
}

void GPIOClass::protocol_event(gpio_evt_id_t evt_id) {
    if (!this->_gpio_protocol) {
        return;
    }
    gpio_sig_id_t sig = START_EVT;
    this->_protocol_send_raw(sig, (int)evt_id);
}

//void M0N0_System::turn_on_gpio_protocol() {
//    this->log_debug("Enabling GPIO protocol");
//    this->_gpio_protocol = true;
//    this->gpio->set_data(0x0); // set to zero
//    this->gpio->set_direction(0xF); // set all as output
//}
//
//void M0N0_System::turn_off_gpio_protocol() {
//    this->log_debug("disabling GPIO protocol");
//    this->_gpio_protocol = false;
//    this->gpio->set_direction(0x0); // set all as input
//}



void SPIClass::set_slave(SPI_SS_t slave_id) {
	// Configures SPI Slave Select in the SPI CTRL reg
	// CRTL.3,CTRL.4,CTRL.5,CRTL.6 are SS0, SS1, SS2 and SS3, respectively
	// CRTL.7 is enable.
    uint8_t en = 1;
    if (slave_id == DESELECT) {
        en = 0;
    }
	this->write(
        SPI_CONTROL_REG,
        SPI_R05_ENABLE_MASK_BIT_MASK,
        en);
	this->write(
            SPI_CONTROL_REG,
            SPI_R05_CHIP_SELECT_BIT_MASK,
            (uint32_t)slave_id);
}

SPI_SS_t SPIClass::get_slave() {
	// Configures SPI Slave Select in the SPI CTRL reg
	// CRTL.3,CTRL.4,CTRL.5,CRTL.6 are SS0, SS1, SS2 and SS3, respectively
	// CRTL.7 is enable.
	return (SPI_SS_t)this->read(
            SPI_CONTROL_REG,
            SPI_R05_CHIP_SELECT_BIT_MASK);
}

void SPIClass::write(uint32_t address, uint32_t data)  {
#ifdef EXTRA_CHECKS
    if (this->_is_autosampling) {
        M0N0_System::error("Cannot use SPI with autosampling enabled");     
    }
#endif
    RegClass::write(address, data);
}

void SPIClass::write(
        uint32_t address,
        uint32_t mask,
        uint32_t data)  {
#ifdef EXTRA_CHECKS
    if (this->_is_autosampling) {
        M0N0_System::error("Cannot use SPI with autosampling enabled");     
    }
#endif
    RegClass::write(address, mask, data);
}


uint8_t SPIClass::write_byte(uint8_t data) {
    /*takes care of the behavior of the HW block*/
    this->write(
            SPI_DATA_WRITE_REG,
            data);
    this->write(
            SPI_COMMAND_REG,
            1);
	__NOP();
	__NOP();
    while (this->read(SPI_STATUS_REG));
    uint8_t temp = (uint8_t)this->read(SPI_DATA_READ_REG);
    // next while added to block execution until PCSM updated
    // note that PCSM acually updates later due to RTC cycle delays
    while (this->read(SPI_STATUS_REG)); 
    return temp;
}

uint8_t SPIClass::write_byte(SPI_SS_t slave_id, uint8_t data) {
    this->set_slave(slave_id);
    return write_byte(data);
}

void SPIClass::pcsm_write(uint8_t address, uint32_t data) {
#ifdef EXTRA_CHECKS
    if (data > 16777216) { // 2^24
        this->_error_f("PCSM data too large");
    }
#endif
    // set mode about
    uint8_t orig_mode = this->get_mode();
    this->set_mode(0);
    this->set_slave(kPcsmSS);
    this->write_byte(address);
    this->write_byte(data >> 16);
    this->write_byte(data >> 8);
    this->write_byte(data);
    SPI_SS_t ss_deselect = DESELECT;
    this->set_slave(ss_deselect);
    this->set_mode(orig_mode);
}

RTCTimer::RTCTimer() {
    this->_start_ticks = 0;
    this->_interval = 0;
}

void RTCTimer::reset() {
    M0N0_System* temp_sys = M0N0_System::get_sys();
    this->_start_ticks = temp_sys->get_rtc();
}

uint64_t RTCTimer::get_cycles() {
    M0N0_System* temp_sys = M0N0_System::get_sys();
    return temp_sys->get_rtc() - this->_start_ticks;
}

float RTCTimer::get_us() {
    return (this->get_cycles() * M0N0_System::kRtcPeriodUs);
}

void RTCTimer::set_interval(uint64_t interval_cycles) {
    this->_interval = interval_cycles;
}

void RTCTimer::set_interval_ms(uint32_t interval_ms) {
    this->_interval = interval_ms * M0N0_System::kRtcOneMsTicks;
}

bool RTCTimer::check_interval() {
    M0N0_System* temp_sys = M0N0_System::get_sys();
    if ((temp_sys->get_rtc() - this->_start_ticks) >= this->_interval) {
        return true;
    }
    return false;
}

void RTCTimer::wait() {
    this->reset(); // reset before, to take into account time for next code
    while (1==1) {
        if (this->check_interval()) {
            return; 
        }
    }
}

// Reduces the DVFS to the minimum level and returns at exit
// note that the level changes after a predefined amount of time
void RTCTimer::wait_lp() {
    this->reset(); // reset before, to take into account time for next code
    M0N0_System* sys = M0N0_System::get_sys();
    uint8_t orig_perf = sys->get_perf();
    sys->set_perf(0);
    while (1) {
        if (this->check_interval()) {
            sys->set_perf(orig_perf);
            return; 
        }
    }
}

void RTCTimer::wait_lp_inttimer() {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->enable_pcsm_interrupt_timer_rtc_ticks(this->_interval, NULL);
    uint8_t orig_perf = sys->get_perf();
    sys->set_perf(0);
    sys->clear_cpu_deepsleep(); // just in case it is set before
    __WFI();
    sys->disable_pcsm_interrupt_timer();
    sys->set_perf(orig_perf);
}

CircBuffer::CircBuffer(
                uint32_t* array,
                uint32_t size,
                uint32_t shram_address,
                bool allow_overwrite,
                Handler_Func full_error_func,
                Handler_Func empty_error_func,
                Handler_Func read_error_func) {
    this->_size = size;
    this->_buffer = array;
    this->_shram_address = shram_address;
    this->_allow_overwrite = allow_overwrite;
    this->_full_error_callback = full_error_func;
    this->_empty_error_callback = empty_error_func;
    this->_read_error_callback = read_error_func;
    this->reset();
}

void CircBuffer::reset(void) {
    this->_head = 0;
    this->_tail = 0;
    this->_full = false;
    this->_total_appends = 0;
    this->_total_removes = 0;
}

bool CircBuffer::is_empty(void) {
    return (this->_head == this->_tail) && (!this->_full);
}

bool CircBuffer::is_full(void) {
    return this->_full;
}

uint32_t CircBuffer::get_capacity(void) {
    return this->_size;
}

uint32_t CircBuffer::get_length(void) {
    if (this->_full) {
        return this->_size;
    } 
    if (this->_head >= this->_tail) {
        return this->_head - this->_tail; 
    } else {
        return this->_head + this->_size - this->_tail;
    }
}

// Returns true if added, false if not
bool CircBuffer::append(uint32_t item) {
    M0N0_System* sys = M0N0_System::get_sys(); 
    if ((!this->_full) || this->_allow_overwrite) {
        // not full, or full but overwrite is allowed 
        this->_buffer[this->_head] = item;
        if (this->_full) { // tail is also advanced if full
            // using module to reset to zero if max size reached
            this->_tail = (this->_tail + 1) % this->_size;
        }
        this->_head = (this->_head + 1) % this->_size;
        this->_full = (this->_head == this->_tail);
        this->_total_appends = this->_total_appends + 1;
        return true;
    } else {
        // full, no overwrite
        sys->log_debug("Buffer FULL");
        if (this->_full_error_callback != NULL) {
            this->_full_error_callback();
        }
        return false;
    }
}

bool CircBuffer::remove(uint32_t* data) {
    if (!this->is_empty()) {
        *data = this->_buffer[this->_tail]; 
        this->_full = false;
        this->_tail = (this->_tail + 1) % this->_size;
        this->_total_removes++;
        return true;
    } 
    // empty
    if (this->_empty_error_callback != NULL) {
        this->_empty_error_callback();
    }
    return false;
}

uint32_t CircBuffer::get_total_appends(void) {
    return this->_total_appends;    
}

// Not implemented:
//uint32_t CircBuffer::get_total_removes(void) {
//    return this->_total_removes;    
//}

bool CircBuffer::read(uint32_t position, uint32_t* data) {
    // tail is zero, head is max
    if (this->is_empty()) {
        if (this->_read_error_callback != NULL) {
            this->_read_error_callback();
        }
        return false;
    }
    if (position >= this->get_capacity()) {
        if (this->_read_error_callback != NULL) {
            this->_read_error_callback();
        }
        return false;
    }
    
    if (this->_head > this->_tail) {
        *data = this->_buffer[this->_tail + position];
        return true; 
    } else {
        if ((position + this->_tail) < this->_size) {
            *data = this->_buffer[this->_tail + position];
            return true;
        } else {
            *data = this->_buffer[position - (this->_size - this->_tail)];
            return true; // starts from 0
        }
    }
}

void CircBuffer::to_array(uint32_t* array) {
    for (uint32_t i = 0; i < this->get_length(); i++) {
        uint32_t* data = 0;
        if (!this->read(i, data)) {
            M0N0_System* sys = M0N0_System::get_sys();
            sys->log_error("Data read failed in CircBuffer::to_array");
        }
        array[i] = *data;
    }
}

// For debugging
void CircBuffer::print(void) {
    M0N0_System* sys = M0N0_System::get_sys(); 
    sys->print("--- Printing CircBuffer ---\n");
    sys->print("is_empty: %d, is_full: %d, len: %d\n",
            this->is_empty(),
            this->is_full(),
            this->get_length());
    for (uint32_t i = 0; i < this->_size; i++) {
        sys->print(" - %02d - %8d ", i, this->_buffer[i]);
        if (i == this->_head) {
            sys->print(" <H> ");
        }
        if (i == this->_tail) {
            sys->print(" <T> ");
        }
        sys->print("\n");
    }
    this->print_array();
    sys->print("--- ------------------- ---\n");
}

void CircBuffer::print_raw_buffer(void) {
    M0N0_System* sys = M0N0_System::get_sys(); 
    sys->print("[ ");
    for (uint32_t i = 0; i < this->get_capacity(); i++) {
        //sys->print("%02d: %02d,  ", i, this->_buffer[i]);
        sys->print(" %03d,  ", this->_buffer[i]);
    }
    sys->print("]\n");
}

void CircBuffer::print_array(void) {
    M0N0_System* sys = M0N0_System::get_sys(); 
    sys->print("[ ");
    for (uint32_t i = 0; i < this->get_length(); i++) {
        uint32_t data = 0;
        this->read(i, &data);
        //sys->print("%02d: %02d,  ", i, data);
        sys->print("%03d: %03d,    ", this->get_sample_count(i), data);
    }
    sys->print("]\n");
}


void CircBuffer::send_via_adp(void) {
    M0N0_System* sys = M0N0_System::get_sys(); 
    for (uint32_t i = 0; i < this->get_length(); i++) {
        uint32_t data = 0;
        this->read(i, &data);
        //sys->print("%02d: %02d,  ", i, data);
        sys->print("\n0x%08X", data);
    }
}

/*
 * (old) HEADER bits:  | 31 | 30 - 16 | 15 - 1 |  0 |
 *                     | x  |   _h    |   _t   | _f |
 *
 *  x=unused, _h = _head, _t = _tail, _f = _full
 */

/*
 * New header is the total appends (only)
 */

void CircBuffer::store_to_shram() {
    M0N0_System* sys = M0N0_System::get_sys();
    uint32_t length = this->get_length(); // NOT size
    sys->shram->write(this->_shram_address, this->_total_appends);
    sys->shram->write(this->_shram_address+4, length);
    for (uint16_t i = 0; i < length; i++) {
        uint32_t data = 0;
        this->read(i, &data);
        sys->shram->write(this->_shram_address+(i*4)+(8), data);
    }
//    sys->log_debug("SHRAM save. is_full: %d, head: %d, tail: %d, len: %d",
//            this->_full,
//            this->_head,
//            this->_tail,
//            length);
}

void CircBuffer::load_from_shram() {
    M0N0_System* sys = M0N0_System::get_sys();
    this->reset();
    uint32_t length = sys->shram->read(this->_shram_address+4); // NOT size
    for (uint16_t i = 0; i < length; i++) {
        uint32_t addr = this->_shram_address+(i*4)+8;
        uint32_t data = sys->shram->read(addr);
        this->append(data);
    }
    // must be done *after* re-adding:
    this->_total_appends = sys->shram->read(this->_shram_address);
//    sys->log_debug("SHRAM restore. is_full: %d, head: %d, tail: %d, len: %d",
//            this->_full,
//            this->_head,
//            this->_tail,
//            length);
}


uint32_t CircBuffer::get_sample_count(uint32_t position) {
    return this->_total_appends - (this->get_length() - position);
}


