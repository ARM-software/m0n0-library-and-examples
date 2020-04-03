
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

#include "m0n0.h"
#include <cstdarg>

extern "C" {
    #include "m0n0_defs.h"
    #include "m0n0_printf.h"
    #include "interrupts.h"
}

#define ADP_COMMAND_ID "3d7db2ae"

// ---------- M0N0 System ---------- //
#ifdef M0N0_HEAP
M0N0_System* M0N0_System::_instance = NULL;
#endif

M0N0_System::M0N0_System() 
: 
        _ctrl(
                CONTROL_BASE_ADDR, // base address
                false,
                CONTROL_SIZE,
                REG_MEM_READ_WRITE,
                &M0N0_read,
                &M0N0_write,
                &M0N0_read_bit_group,
                &M0N0_write_bit_group,
                &M0N0_System::error,
                &M0N0_System::debug),
        _status(
                STATUS_BASE_ADDR, // base address
                false,
                STATUS_SIZE,
                REG_MEM_READ,
                &M0N0_read,
                NULL,
                &M0N0_read_bit_group,
                NULL,
                &M0N0_System::error,
                &M0N0_System::debug),
        _aes(
                AES_BASE_ADDR, // base address
                false,
                AES_SIZE,
                REG_MEM_READ_WRITE,
                &M0N0_read,
                &M0N0_write,
                &M0N0_read_bit_group,
                &M0N0_write_bit_group,
                &M0N0_System::error,
                &M0N0_System::debug),
        _spi(
                SPI_BASE_ADDR, // base address
                false,
                SPI_SIZE,
                REG_MEM_READ_WRITE,
                &M0N0_read,
                &M0N0_write,
                &M0N0_read_bit_group,
                &M0N0_write_bit_group,
                &M0N0_System::error,
                &M0N0_System::debug),
        _gpio(
                GPIO_BASE_ADDR, // base address
                false,
                GPIO_SIZE,
                REG_MEM_READ_WRITE,
                &M0N0_read,
                &M0N0_write,
                &M0N0_read_bit_group,
                &M0N0_write_bit_group,
                &M0N0_System::error,
                &M0N0_System::debug),
        _shram(
                MEM_MAP_SHRAM_BASE, // base address
                true,
                MEM_MAP_SHRAM_SIZE,
                REG_MEM_READ_WRITE,
                &M0N0_read,
                &M0N0_write,
                &M0N0_read_bit_group,
                &M0N0_write_bit_group,
                &M0N0_System::error,
                &M0N0_System::debug)
{
    LOG_LEVEL_t ll = DEFAULT_LOG_LEVEL;
    this->_log_level = ll;
    this->_handler_extwake = NULL;
    this->_handler_systick = NULL;
    this->_handler_pcsm_inttimer = NULL;
    this->_handler_autosample = NULL;
    this->autosample_disable_flag = false;
    this->_adp_tx_name = "null"; 
    this->ctrl = &(this->_ctrl);
    this->status = &(this->_status);
    this->aes = &(this->_aes);
    this->spi = &(this->_spi);
    this->gpio = &(this->_gpio);
    this->shram = &(this->_shram);
    // test whether VBAT PoR was issued
    // There is no built in way to detect VBAT PoR
    // This functionality is added by exploiting the fact that the ROM bank
    // power on "wakeup" delay is set in the PCSM (code_ctrl), can be read
    // from Status Register 7, and is 6 bits (larger than ever required). 
    uint32_t rom_poweron_delay = this->status->read(
            STATUS_STATUS_7_REG,
            STATUS_R07_ROM_WAKEUP_DELAY_BIT_MASK);
    if (rom_poweron_delay == 32) {
        // power-on reset value
        _vbat_por = true;
    } else {
        // saved value from before 
        _vbat_por = false;
    }
    // ROM power-on delay to 5
    // need to maintain the other bits in the PCSM, i.e. the memory remap
    uint32_t temp_code_ctrl = this->status->read(
            STATUS_STATUS_7_REG,
            STATUS_R07_MEMORY_REMAP_BIT_MASK);
    temp_code_ctrl |= (5 << 3);
    this->spi->pcsm_write(PCSM_CODE_CTRL_REG, temp_code_ctrl);
}

#ifdef M0N0_HEAP
#else

M0N0_System &M0N0_System::sys_instance() {
    static M0N0_System unique_instance;
    return unique_instance;
}
#endif

void M0N0_System::set_log_level(LOG_LEVEL_t log_level) {
    this->_log_level = log_level;
}

void M0N0_System::_shutdown_cleanup(void) {
    if (this->spi->get_is_autosampling()) {
        this->disable_autosampling(); 
    }
}

int M0N0_System::log_debug(const char *fmt, ...) {
#ifdef SUPPRESS_STDOUT
    return -1;
#endif
    LOG_LEVEL_t temp = DEBUG;
    if (temp < this->_log_level) {
        return -1;
    }
    if (!M0N0_System::_is_deve()) {
        return -1;
    }
    std::string fmt_str(fmt);
    fmt_str = "DEBUG: "+fmt_str+"\n";
    char const *new_fmt = &(fmt_str[0]);
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = simple_vsprintf(NULL, new_fmt, ap);
    va_end(ap);
    return r;
}

int M0N0_System::log_info(const char *fmt, ...) {
#ifdef SUPPRESS_STDOUT
    return -1;
#endif
    LOG_LEVEL_t temp = INFO;
    if (temp < this->_log_level) {
        return -1;
    }
    if (!M0N0_System::_is_deve()) {
        return -1;
    }
    std::string fmt_str(fmt);
    fmt_str = "INFO:  "+fmt_str+"\n";
    char const *new_fmt = &(fmt_str[0]);
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = simple_vsprintf(NULL, new_fmt, ap);
    va_end(ap);
    return r;
}

int M0N0_System::log_warn(const char *fmt, ...) {
#ifdef SUPPRESS_STDOUT
    return -1;
#endif
    LOG_LEVEL_t temp = WARN;
    if (temp < this->_log_level) {
        return -1;
    }
    if (!M0N0_System::_is_deve()) {
        return -1;
    }
    std::string fmt_str(fmt);
    fmt_str = "WARN:  "+fmt_str+"\n";
    char const *new_fmt = &(fmt_str[0]);
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = simple_vsprintf(NULL, new_fmt, ap);
    va_end(ap);
    return r;
}

int M0N0_System::log_error(const char *fmt, ...) {
#ifdef SUPPRESS_STDOUT
    return -1;
#endif
    LOG_LEVEL_t temp = ERROR;
    if (temp < this->_log_level) {
        return -1;
    }
    if (!M0N0_System::_is_deve()) {
        return -1;
    }
    std::string fmt_str(fmt);
    fmt_str = "ERROR: "+fmt_str+"\n";
    char const *new_fmt = &(fmt_str[0]);
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = simple_vsprintf(NULL, new_fmt, ap);
    va_end(ap);
    return r;
}

uint8_t M0N0_System::_get_raw_perf() {
    return (uint8_t)this->status->read(
            STATUS_STATUS_7_REG,
            STATUS_R07_PERF_BIT_MASK);
}

uint8_t M0N0_System::get_perf() {
    return this->_inv_perf_lookup[(uint8_t)this->status->read(
            STATUS_STATUS_7_REG,
            STATUS_R07_PERF_BIT_MASK)];
}

void M0N0_System::_set_raw_perf(uint8_t raw_perf) {
#ifdef EXTRA_CHECKS
    if (raw_perf < 16 || raw_perf > 31) {
        M0N0_System::error("Invalid raw perf");
    }
#endif
    this->spi->pcsm_write(PCSM_PERF_CTRL_REG, raw_perf);
}

void M0N0_System::set_perf(uint8_t perf) {
#ifdef EXTRA_CHECKS
    if (perf > 15) {
        M0N0_System::error("Invalid perf");
    }
#endif
    this->_set_raw_perf(_perf_lookup[perf]);
    // This makes the device wait until the perf has actually been updated:
    // while(this->_get_raw_perf()!=_perf_lookup[perf]);
    // note that the actual Voltage and frequency does not change immediately 
    // due to operation of the IVR
}


uint64_t M0N0_System::get_rtc() {
    uint64_t rtc = this->status->read(
            STATUS_STATUS_4_REG,
            STATUS_R04_RTC_MSBS_BIT_MASK);
    rtc = rtc << 32;
    rtc |= this->status->read(
            STATUS_STATUS_2_REG,
            STATUS_R02_RTC_LSBS_BIT_MASK);
    return rtc;
}

float M0N0_System::get_rtc_us() {
    return (this->get_rtc() * kRtcPeriodUs) ;
}

bool M0N0_System::is_rtc_real_time() {
    return this->status->read(
            STATUS_STATUS_7_REG,
            STATUS_R07_REAL_TIME_FLAG_BIT_MASK);
}

void M0N0_System::sleep_rtc(uint64_t rtc_ticks) {
    uint64_t start = this->get_rtc();
    while ((this->get_rtc() - start) < rtc_ticks) {
        // wait
    }
}

void M0N0_System::sleep_ms(uint32_t time_ms) {
    uint64_t time_raw = time_ms * kRtcOneMsTicks;
    uint64_t start = this->get_rtc();
    while ((this->get_rtc() - start) < time_raw) {
        // wait
    }
}


/*
 * STATIC
 */

// Does not use reg objects (req for printf before they are created)
bool M0N0_System::_is_deve() {
    return M0N0_read_bit_group(
            STATUS_STATUS_7_REG,
            STATUS_R07_DEVE_CORE_BIT_MASK);
}

int M0N0_System::print(const char *fmt, ...) {
#ifdef SUPPRESS_STDOUT
    return -1;
#endif
    if (!M0N0_System::_is_deve()) {
        return -1;
    }
    int r;
    va_list ap;
    va_start(ap, fmt);
    r = simple_vsprintf(NULL, fmt, ap);
    va_end(ap);
    return r;
}

void M0N0_System::error(const char *message) {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_error(message);
    throw message;
}

void M0N0_System::debug(const char *message) {
    M0N0_System* sys = M0N0_System::get_sys();
    sys->log_debug(message);
}


int M0N0_System::run_testcase(testcase_id_t tc, uint32_t verbose, uint64_t repeat_delay) {
    this->log_debug("TC: %d, v: %d, rpt dly: %d",
            (int)tc, 
            verbose,
            (uint32_t)repeat_delay);
    return tc_funcs_run_testcase(tc, verbose, repeat_delay);
}


void M0N0_System::wait_for_adp(
        uint32_t timeout_ms, // timeout of zero means forever
        uint32_t verbose) {
  // Poll ADP 
  /* Use CTRL5
   * [31:16] RTC repeat delay (multiplied by 4096)
   *         (0 bypasses all delay code)
   * [15:8]  testcase_id
   * [7:0]   UNUSED
   * [0]   strobe
   */
  // Initialise
    this->ctrl->write(CONTROL_CTRL_5_REG, 0);
    uint64_t escape_timeout = timeout_ms * this->kRtcOneMsTicks;
    uint64_t rtc_start = this->get_rtc();
    uint8_t wait_for_rtc_flag = 1; // wait for RTC
    this->log_debug("rtc start 0x%x%x\n",
        (uint32_t)(rtc_start >> 32),(uint32_t)rtc_start);
    // signal waiting for ADP for first time
    testcase_id_t wfa_tc = WAIT_FOR_ADP; // for gpio printing
    this->print("Waiting for ADP direction...\n");
    this->gpio->protocol_tc_start(wfa_tc);
    while (1) {
        // check strobe
        uint32_t ctrl5 = this->ctrl->read(CONTROL_CTRL_5_REG); 
        if (ctrl5 & 0x01) { // if strobe 1
            // turn off KWS RTC check
            wait_for_rtc_flag = 0;
            // run testcase
            testcase_id_t tc_id = (testcase_id_t)((ctrl5 >> 8) & 0xFF);
            uint64_t user_rtc_delay = (uint64_t)((ctrl5 & 0xFFFF0000) >> 16) ;
            user_rtc_delay = user_rtc_delay << 12; // multiply by 4096
            this->print("Strobe. TCID: %d, Repeat Delay: 0x%x%x\n",
            tc_id,(uint32_t)(user_rtc_delay >> 32),(uint32_t)user_rtc_delay);
            run_testcase(tc_id,verbose,user_rtc_delay);
            //IMPORTANT, now reset ctrl5 to 0
            this->ctrl->write(CONTROL_CTRL_5_REG, 0x00000000); // strobe is 0
            // Re-send 'wait for GPIO message'
            this->print("Waiting for ADP direction...\n");
            this->gpio->protocol_tc_start(wfa_tc);
        }
        // check timeout
        if (wait_for_rtc_flag && timeout_ms > 0) {
          uint64_t rtc_cur = this->get_rtc();
          if ((rtc_cur - rtc_start) >= escape_timeout) {
            this->print("Exiting WFADP\n");
            break;
          }
        }
    }
    this->gpio->protocol_tc_end(wfa_tc);
}

void M0N0_System::power_off_roms() {
    this->ctrl->write(CONTROL_CTRL_2_REG, 0);
}

void M0N0_System::set_cpu_deepsleep() {
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}

void M0N0_System::clear_cpu_deepsleep() {
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
}

void M0N0_System::_set_rtc_wakeup(uint64_t rtc_ticks) {
#ifdef EXTRA_CHECKS
    if (rtc_ticks > 281474976710655) { // 2^48-1
        M0N0_System::error("RTCWKP Oflow");
    }
#endif
    uint32_t msbs = (uint32_t)((rtc_ticks>>24))&0x00FFFFFF;
    // 24 MSBS
    this->spi->pcsm_write(PCSM_RTC_WKUP1_REG, msbs);
    uint32_t lsbs = ((uint32_t)rtc_ticks)&0x00FFFFFF;
    // 24 LSBs
    this->spi->pcsm_write(PCSM_RTC_WKUP0_REG, lsbs);
}

void M0N0_System::_clear_rtc_wakeup() {
    // 24 MSBS
    this->spi->pcsm_write(PCSM_RTC_WKUP1_REG, 0);
    // 24 LSBs
    this->spi->pcsm_write(PCSM_RTC_WKUP0_REG, 0);
}

void M0N0_System::timed_shutdown(uint64_t rtc_ticks) {
#ifdef EXTRA_CHECKS
    if (rtc_ticks == 0) {
        M0N0_System::error("RTCWKP 0Err");
    }
#endif
    this->_set_rtc_wakeup(rtc_ticks);
    this->log_debug("T.Shtdwn (%d rtc tks)", rtc_ticks);
    this->set_cpu_deepsleep();
    __WFI();
}

void M0N0_System::timed_shutdown_ms(const uint32_t time_ms) {
#ifdef EXTRA_CHECKS
    if (time_ms == 0) {
        M0N0_System::error("RTCWKP 0Err");
    }
#endif
    uint32_t time_raw = (uint32_t)time_ms * (uint32_t)kRtcOneMsTicks;
    this->log_debug("T.Shtdwn (%d ms, %d rtc tks)",
            time_ms,
            time_raw);
    this->_set_rtc_wakeup(time_raw);
    this->set_cpu_deepsleep();
    __WFI();
}

void M0N0_System::deep_shutdown() {
    this->log_debug("D. Shtdwn");
    this->_clear_rtc_wakeup(); // ensure RTC wakeup is zero
    this->set_cpu_deepsleep();
    __WFI();
}

char M0N0_System::wait_read_stdin() {
    return M0N0_read_stdin();    
}

void M0N0_System::set_recommended_settings() {
    this->log_debug("Recomm. sys settinngs");
    // enable RTC FBB
    this->spi->pcsm_write(PCSM_RTC_CTRL1_REG, 0x27 | (1<<3)); // PoR, but [3]=1
    // set SHRAM delay to 1
    this->ctrl->write(CONTROL_CTRL_4_REG, CONTROL_R04_SHRAM_DELAY_BIT_MASK, 1);
    // set dataram delay to 1
    this->ctrl->write(CONTROL_CTRL_4_REG, CONTROL_R04_DATARAM_DELAY_BIT_MASK, 1);
    // Note that the ROM poweron delay is set in the constructor
}

void M0N0_System::enable_extwake_interrupt(Handler_Func f) {
    this->_handler_extwake = f;
    __NVIC_EnableIRQ(Interrupt6_IRQn);
}

void M0N0_System::disable_extwake_interrupt() {
    this->_handler_extwake = NULL;
    __NVIC_DisableIRQ(Interrupt6_IRQn);
}

extern "C" void hand_extwake() {
    M0N0_System* sys = M0N0_System::get_sys();
    if (sys->_handler_extwake == NULL) {
        M0N0_System::debug("ewake hndlr null");
        return;
    }
    return sys->_handler_extwake();
}

extern "C" void hand_systick() {
    M0N0_System* sys = M0N0_System::get_sys();
    if (sys->_handler_systick == NULL) {
        M0N0_System::debug("stick hndlr null");
        return;
    }
    return sys->_handler_systick();
}

extern "C" void hand_autosample() {
    M0N0_System* sys = M0N0_System::get_sys();
    if (sys->autosample_disable_flag) {
        // DISABLE autosample
        sys->spi->disable_autosampling();
        return;
    }
    if (sys->_handler_autosample == NULL) {
        M0N0_System::debug("asample hndlr null");
        return;
    }
    return sys->_handler_autosample();
}

extern "C" void hand_pcsm_timer() {
    M0N0_System* sys = M0N0_System::get_sys();
    if (sys->_handler_pcsm_inttimer == NULL) {
        M0N0_System::debug("inttimer hndlr null");
        return;
    }
    return sys->_handler_pcsm_inttimer();
}

bool M0N0_System::is_vbat_por(void) {
    return this->_vbat_por; 
}

bool M0N0_System::is_extwake(void) {
    return this->status->read(
        STATUS_STATUS_7_REG,
        STATUS_R07_EXT_WAKE_BIT_MASK); 
}

void M0N0_System::_enable_systick(uint32_t ticks) {
    SysTick->CTRL = 0; 
    SysTick->VAL = ticks; /* Load the SysTick Counter Value */
    SysTick->LOAD = ticks; /* Load the SysTick Counter Value */
    SysTick->CTRL = (SysTick_CTRL_TICKINT_Msk   | 
                     SysTick_CTRL_ENABLE_Msk) |
                     SysTick_CTRL_CLKSOURCE_Msk;
}

//* Prints basic system information (for debugging and demonstration)
void M0N0_System::print_info(void) {
    this->log_info("Sys status:") ;
    this->log_info("CPUID: 0x%X",SCB->CPUID) ;
    this->log_info("DEVE?: %d", M0N0_System::_is_deve());
    this->log_info("DVFS Level: %d, HW ID: %d", this->get_perf(),
             this->_get_raw_perf());
    this->log_debug("Fault Status Register, CFSR: %X\n", (SCB->CFSR));
    this->log_info("is RTC real-time?: %d, VBAT PoR?: %d",
            this->is_rtc_real_time(), this->is_vbat_por());
    uint64_t rtc_cycles = this->get_rtc();
    uint32_t rtc_us = (uint32_t)(rtc_cycles*kRtcPeriodUs); // Inefficient
    this->log_info("RTC Cycles: 0x%X%X (%u us, %u seconds)",
            (uint32_t)(rtc_cycles >> 32),
            (uint32_t)(rtc_cycles),
            rtc_us,
            (rtc_us/1000000));
    this->log_info("Memory Remap: %d", this->status->read(
            STATUS_STATUS_7_REG,
            STATUS_R07_MEMORY_REMAP_BIT_MASK));
    this->log_info("CTRL2 (rom power status): 0x%X", this->ctrl->read(
            CONTROL_CTRL_2_REG));
    this->log_info("Battery Monitor: [under: %d, over: %d]", 
            this->status->read(
                    STATUS_STATUS_7_REG,
                    STATUS_R07_BATMON_UNDER_BIT_MASK),
            this->status->read(
                    STATUS_STATUS_7_REG,
                    STATUS_R07_BATMON_OVER_BIT_MASK));
    this->log_info("Status 7: 0x%X",this->status->read(
            STATUS_STATUS_7_REG));
}


void M0N0_System::generate_hardfault(void) {
    this->log_debug("Generating hard fault");
    SCB->CCR |= 0x10;
    int a = 0;
    int b = 5/a;
    this->log_info("b: %d",b);
}

uint32_t M0N0_System::estimate_tcro(void) {
    uint64_t ticks = 10000000;
    uint64_t wait_rtcs = 10*kRtcOneMsTicks; // 10 ms
    this->disable_systick();
    SysTick->CTRL = 0; 
    SysTick->VAL = ticks; /* Load the SysTick Counter Value */
    SysTick->LOAD = ticks; /* Load the SysTick Counter Value */
    uint64_t rtc_start = this->get_rtc();
    SysTick->CTRL = (0   |  // no interrupt
                     SysTick_CTRL_ENABLE_Msk) |
                     SysTick_CTRL_CLKSOURCE_Msk;
    while (this->get_rtc() < (rtc_start + wait_rtcs))  {
        // wait
    }
    uint64_t elapsed_ticks = ticks - SysTick->VAL;
    //uint64_t rtc_end = this->get_rtc();
    SysTick->CTRL = 0; 
    return ((elapsed_ticks * 100)/1000);
}

void M0N0_System::enable_systick(uint32_t ticks, Handler_Func f) {
    this->_handler_systick = f;
    __NVIC_EnableIRQ(SysTick_IRQn);
    this->_enable_systick(ticks);
}

void M0N0_System::disable_systick(void) {
    SysTick->CTRL = 0; 
    __NVIC_DisableIRQ(SysTick_IRQn);
    this->_handler_systick = NULL;
}

void M0N0_System::_set_inttimer(uint32_t rtc_ticks) {
    if (rtc_ticks == 0) {
        this->spi->pcsm_write(PCSM_INTTIMER0_REG, 0);
    } else if (rtc_ticks > 1) {
        this->spi->pcsm_write(PCSM_INTTIMER0_REG, rtc_ticks - 1);
    } else {
        M0N0_System::error("inttimer0 RTC ticks bust be 0 or >= 2");    
    }
}

void M0N0_System::enable_pcsm_interrupt_timer_ms(uint32_t interval_ms, Handler_Func f) {
    this->_handler_pcsm_inttimer = f;
    this->_set_inttimer(interval_ms * kRtcOneMsTicks);
    __NVIC_EnableIRQ(Interrupt5_IRQn);
}

void M0N0_System::enable_pcsm_interrupt_timer_rtc_ticks(uint32_t rtc_ticks, Handler_Func f) {
    this->_handler_pcsm_inttimer = f;
    this->_set_inttimer(rtc_ticks);
    __NVIC_EnableIRQ(Interrupt5_IRQn);
}

void M0N0_System::disable_pcsm_interrupt_timer(void) {
    __NVIC_DisableIRQ(Interrupt5_IRQn);
    this->_set_inttimer(0);
}

void M0N0_System::enable_autosampling_ms(uint32_t interval_ms, Handler_Func f) {
    this->_handler_autosample = f;
    this->_set_inttimer(interval_ms * kRtcOneMsTicks);
    this->spi->enable_autosampling();
    __NVIC_EnableIRQ(Interrupt1_IRQn);
}

void M0N0_System::enable_autosampling_rtc_ticks(uint32_t rtc_ticks, Handler_Func f) {
    this->_handler_autosample = f;
    if (rtc_ticks < 2) {
        M0N0_System::error("inttimer0 RTC ticks bust be >= 2");    
    }
    this->_set_inttimer(rtc_ticks );
    __NVIC_EnableIRQ(Interrupt1_IRQn);
    this->spi->enable_autosampling();
}

void M0N0_System::disable_autosampling_wait(void) {
    this->autosample_disable_flag = true;
    this->log_debug("Disabling autosampling, waiting for next IQR...");
    while (this->autosample_disable_flag) {
        //__WFI;
    }
    __NVIC_DisableIRQ(Interrupt1_IRQn);
    this->_set_inttimer(0);
    this->log_debug("Autosample disabled");
}

void M0N0_System::disable_autosampling(void) {
    this->spi->disable_autosampling();
    __NVIC_DisableIRQ(Interrupt1_IRQn);
    this->_set_inttimer(0);
    this->log_debug("Autosample disabled");
}


void M0N0_System::adp_tx_start(const char *name) {
    this->_adp_tx_name = name; 
    this->log_info("Starting TX...");
    this->print("\n");
    this->print(ADP_COMMAND_ID);
    this->print("_tx_start<<");
    this->print(this->_adp_tx_name);
    this->print(">>"); // no newline
}

void M0N0_System::adp_tx_end_of_params(void) {
    this->print("\n");
    this->print(ADP_COMMAND_ID);
    this->print("_params_end");
}

void M0N0_System::adp_tx_end(void) {
    this->print("\n");
    this->print(ADP_COMMAND_ID);
    this->print("_tx_end<<");
    this->print(this->_adp_tx_name);
    this->print(">>\n");
    this->log_info("Ended transaction");
}

