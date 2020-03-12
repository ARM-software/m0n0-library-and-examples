
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

#include "tc_functions.h"
#include "m0n0.h"

/* some testcases can't be called (only declared for
 * viewing over GPIO, and so have an empty
 * testcase function
 */
Testcase_Func tc_fncs[NUM_TESCASES] = {
  empty_test, // NONE
  empty_test, // INITIALISATION
  empty_test, // WAIT_FOR_ADP
  print_hello, // HELLO
  enable_deep_sleep, // EN_D_SLEEP
  disable_deep_sleep, // DISABLE_D_SLEEP
  call_wfi, // CALL_WFI
  update_sw_perf, // UPDATE_PERF
  echo_stdin, // ECHO_STDIN
  tc_sanity, // SANITY_TC
  tc_aes, // AES_TC
  tc_rtc, // RTC_TC
  tc_perf, // PERF_TC
};

int empty_test(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("--- empty test ---\n");
    return TCPASS;
}

int print_hello(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("Hello World\n");
    return TCPASS;
}

int enable_deep_sleep(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("--- enable deep sleep ---");
    sys->set_cpu_deepsleep();
    return TCPASS;
}

int disable_deep_sleep(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("--- disable deep sleep ---");
    sys->clear_cpu_deepsleep();
    return TCPASS;
}

int call_wfi(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("--- WFI ---");
    __WFI();
    return TCPASS;
}

int update_sw_perf(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("Update perf not yet implemented");
    return TCPASS;
}

int echo_stdin(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    char res[140] = "";
    int count = 0;
    if (verbose) sys->print("--- echo stdin ---");
    while (true) {
        char c = sys->wait_read_stdin();
        if (c == '\n' || count > 139) {
            // print
            sys->print("Res:");
            for (int i = 0; i < count; i++) {
                sys->print("%c",res[i]);
            }
            sys->print("\n");
            count = 0;
        } else {
            res[count] = c;
            //sys->print("SI:%c, Count: %d, Res[count]: %c\n",c,count,res[count]);
            count++;
        }
    }
    return TCPASS;
}

// Begin: System Tests

// utility function:
void print_array(uint32_t* arr, uint32_t length) {
    M0N0_System* sys = M0N0_System::get_sys();
    for (uint32_t i = 0; i < length; i++) {
        sys->log_info("%d: 0x%x",i,*(arr+i));
    }
}

int tc_sanity(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("--- tc_sanity ---\n");
    sys->log_debug("shram delay?: 0x%x",sys->status->read(
            CONTROL_CTRL_4_REG,
            CONTROL_R04_SHRAM_DELAY_BIT_MASK));

    sys->log_debug("dataram delay?: 0x%x",sys->status->read(
            CONTROL_CTRL_4_REG,
            CONTROL_R04_DATARAM_DELAY_BIT_MASK));

    sys->log_debug("coderam delay?: 0x%x",sys->status->read(
            CONTROL_CTRL_4_REG,
            CONTROL_R04_CODERAM_DELAY_BIT_MASK));

    sys->log_debug("isDEVE?: 0x%x",sys->status->read(
            STATUS_STATUS_7_REG,
            STATUS_R07_DEVE_CORE_BIT_MASK));

    sys->log_debug("Raw perf: 0x%x",sys->status->read(
            STATUS_STATUS_7_REG,
            STATUS_R07_PERF_BIT_MASK));

    // read the Control Registers:
    sys->log_info("CTRL_REG_0: 0x%x",sys->ctrl->read(CONTROL_CTRL_0_REG));
    sys->log_info("CTRL_REG_1: 0x%x",sys->ctrl->read(CONTROL_CTRL_1_REG));
    sys->log_info("CTRL_REG_2: 0x%x",sys->ctrl->read(CONTROL_CTRL_2_REG));
    sys->log_info("CTRL_REG_3: 0x%x",sys->ctrl->read(CONTROL_CTRL_3_REG));
    sys->log_info("CTRL_REG_4: 0x%x",sys->ctrl->read(CONTROL_CTRL_4_REG));
    sys->log_info("CTRL_REG_5: 0x%x",sys->ctrl->read(CONTROL_CTRL_5_REG));

    // read the Status Registers:
    sys->log_info("STAT_REG_0: 0x%x",sys->status->read(STATUS_STATUS_0_REG));
    sys->log_info("STAT_REG_1: 0x%x",sys->status->read(STATUS_STATUS_1_REG));
    sys->log_info("STAT_REG_2: 0x%x",sys->status->read(STATUS_STATUS_2_REG));
    sys->log_info("STAT_REG_3: 0x%x",sys->status->read(STATUS_STATUS_3_REG));
    sys->log_info("STAT_REG_4: 0x%x",sys->status->read(STATUS_STATUS_4_REG));
    sys->log_info("STAT_REG_5: 0x%x",sys->status->read(STATUS_STATUS_5_REG));
    sys->log_info("STAT_REG_7: 0x%x",sys->status->read(STATUS_STATUS_7_REG));
    return TCPASS;
}

int tc_aes(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("--- tc_aes ---\n");
     // ROHANKARTHINMEG
    sys->log_info("Starting AES Test");
    sys->log_info("AES control: 0x%x",sys->aes->read(AES_CONTROL_REG));
    sys->log_info("AES status: 0x%x",sys->aes->read(AES_STATUS_REG));
    sys->log_info("Testing encryption function (short)...");
    uint32_t key_256 [8] = {0x524f4841, 0x4e4b4152, 0x5448494b, 0x4d454700,
            0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa}; // ROHANKARTHINMEG
    uint32_t data [4] = {0x57656c63, 0x6f6d6520, 0x746f2041, 0x524d2100};
    uint32_t encr [4] = {0,0,0,0};
    uint32_t decr [4] = {0,0,0,0};
    sys->aes->set_key(key_256);
    sys->aes->encrypt_blocking(data, 4, encr);
    sys->log_info("Encrypted data: ");
    print_array(encr,4);
    sys->aes->decrypt_blocking(encr, 4, decr);
    sys->log_info("Decrypted data: ");
    print_array(decr,4);

    sys->log_info("Completed AES Test");
    return TCPASS;
}

int tc_rtc(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("--- tc_rtc ---\n");
    uint64_t rtc = sys->get_rtc();
    sys->log_info("RTC: 0x%08x%08x", (uint32_t)(rtc >> 32), (uint32_t)(rtc));
    sys->log_info("Microseconds: %d", (uint32_t)sys->get_rtc_us());
    sys->log_info("Read time?: %d", sys->is_rtc_real_time());
    return TCPASS;
}

int tc_perf(uint32_t verbose) {
    M0N0_System* sys = M0N0_System::get_sys();
    if (verbose) sys->print("--- tc_perf ---\n");
    sys->log_info("perf: %d",sys->get_perf());
    sys->log_info("Setting new perf");
    sys->set_perf(3);
    sys->log_info("Perf: %d",sys->get_perf());
    sys->log_info("Perf: %d",sys->get_perf());
    sys->log_info("Perf: %d",sys->get_perf());
    sys->log_info("Perf: %d",sys->get_perf());
    return TCPASS;
}

// End: System Tests


int tc_funcs_run_testcase(testcase_id_t tc, uint32_t verbose, uint64_t repeat_delay) {
  M0N0_System* sys = M0N0_System::get_sys();
  sys->print("\n--- TCID: %d ---\n",tc);
  sys->gpio->protocol_tc_start(tc);
  if (repeat_delay) {
    // Repeat mode:
    // 0, off
    // RTC delay
    uint64_t rtc_start = sys->get_rtc();
    sys->print("Repeat delay: 0x%x%x, RTC Start: 0x%x%x)\n",
          (uint32_t)(repeat_delay>>32),(uint32_t)repeat_delay,(uint32_t)(rtc_start>>32),(uint32_t)rtc_start);
    uint64_t rtc_cur = sys->get_rtc();
    int temp_result = 0; 
    do {
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      temp_result += (tc_fncs[tc])(0);
      rtc_cur = sys->get_rtc();
    } while ((rtc_cur - rtc_start) <= repeat_delay);
    sys->print("Finished power loop, rtc_cur: 0x%x%x)\n",
          (uint32_t)(rtc_cur>>32),(uint32_t)rtc_cur);
    sys->print("Temp result: %d\n",temp_result);
  }
  // 2. run function
  int result = (tc_fncs[tc])(verbose);
  // 3. send GPIO end signal
  if (result == TCPASS) {
    sys->gpio->protocol_event(TC_PASSED);
  } else {
    sys->gpio->protocol_event(TC_FAILED);
  }
  sys->gpio->protocol_tc_end(tc);
  if (result == TCPASS) {
    sys->print("\nTC STATUS:TCPASS\n");
  } else {
    sys->print("\nTC STATUS:TCFAIL\n");
  }
  return result;
}
