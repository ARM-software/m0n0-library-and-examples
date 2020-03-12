
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

#ifndef TC_FUNCTIONS_H
#define TC_FUNCTIONS_H
#include <cstdint>

#define NUM_TESCASES 100

typedef enum  {
  NONE,
  INITIALISATION,
  WAIT_FOR_ADP,
  HELLO,
  EN_D_SLEEP,
  DISABLE_D_SLEEP,
  CALL_WFI,
  UPDATE_PERF,
  ECHO_STDIN,
  SANITY_TC,
  AES_TC,
  RTC_TC,
  PERF_TC
} testcase_id_t;

#define TCPASS 0
#define TCFAIL 1

typedef enum  {
  RESERVED, // End of Transaction
  START_TC, // Start Testcase 
  END_TC, // End Testcase 
  START_EVT // Event occured (no end?)
} gpio_sig_id_t;

typedef enum GPIO_EVT_ID {
  TC_PASSED,
  TC_FAILED,
  TCS_END_ALL_PASS,
  TCS_END_FAIL,
  PROGRAM_END
} gpio_evt_id_t;


typedef int (*Testcase_Func)(uint32_t);

int empty_test(uint32_t verbose);

int print_hello(uint32_t verbose);
int enable_deep_sleep(uint32_t verbose);
int disable_deep_sleep(uint32_t verbose);
int call_wfi(uint32_t verbose);
int update_sw_perf(uint32_t verbose);
int echo_stdin(uint32_t verbose);

void print_array(uint32_t* arr, uint32_t length); // utility function

// Begin: System Tests
int tc_sanity(uint32_t verbose);
int tc_aes(uint32_t verbose);
int tc_rtc(uint32_t verbose);
int tc_perf(uint32_t verbose);
// End: System Tests

int tc_funcs_run_testcase(testcase_id_t tc, uint32_t verbose, uint64_t repeat_delay);

#endif // TC_FUNCTIONS_H
