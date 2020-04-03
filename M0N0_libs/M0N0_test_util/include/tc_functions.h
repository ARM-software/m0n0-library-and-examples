
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

/**
@file
@brief Defines test functions ("testcases") that can be run via ADP
*/

/** Testcase name definitions. Enum value defines its ID; order is important
 * and must match the order of the functions defined tc_fncs (tc_functions.cpp)
 */
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

/** Value returned from testcase when it has passed successfully (test passed)
 */
#define TCPASS 0
/** Value returned from testcase when an error has been detected (test failed)
 */
#define TCFAIL 1

/** GPIO protocol transaction type flags (for simulator testing only)
 */
typedef enum  {
  RESERVED, // End of Transaction
  START_TC, // Start Testcase 
  END_TC, // End Testcase 
  START_EVT // Event occurred (no end?)
} gpio_sig_id_t;


/** GPIO protocol event type flags (for simulator testing only)
 */
typedef enum GPIO_EVT_ID {
  TC_PASSED,
  TC_FAILED,
  TCS_END_ALL_PASS,
  TCS_END_FAIL,
  PROGRAM_END
} gpio_evt_id_t;

/** Generic function definition for all testcases
 */
typedef int (*Testcase_Func)(uint32_t);

/** Empty testcase function
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int empty_test(uint32_t verbose);

/** Simple testcase that prints STDOUT
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int print_hello(uint32_t verbose);
/** Testcase that enables CPU deep sleep flag
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int enable_deep_sleep(uint32_t verbose);
/** Testcase that disables CPU deep sleep flag
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int disable_deep_sleep(uint32_t verbose);
/** Testcase that calls a WFI instruction
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int call_wfi(uint32_t verbose);
/** Not implemented
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int update_sw_perf(uint32_t verbose);
/** Testcase that echos STDIN to STDOUT
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int echo_stdin(uint32_t verbose);

/** Utility function for printing an array via STDOUT

 */
void print_array(uint32_t* arr, uint32_t length); // utility function

/** Testcase for printing register values for quick system checking
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int tc_sanity(uint32_t verbose);
/** Testcase that uses the AES hardware
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int tc_aes(uint32_t verbose);
/** Testcase that uses the RTC counter
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int tc_rtc(uint32_t verbose);
/** Testcase that uses the DVFS control
 *
 * @param verbose Whether the testcase should issue STDOUT (TRUE) or not
 * @return Flag indicating whether the testcase has passed (TCPASS) or failed
 *     (TCFAIL). Note that many testcases are used as utilities rather than
 *     tests so will always return TCPASS
 */
int tc_perf(uint32_t verbose);

/** Function that calls a testcase using the ID enum
  *
  * @param tc The testcase ID (enum, e.g. EN_D_SLEEP)
  * @param verbose The verbose flag to forward to the testcase defining
  *     it should issue STDOUT (TRUE) or not (FALSE)
  * @param repeat_delay If 0, the testcase is run once and the result is
  *     returned. It it is >0, then the testcase is repeated (with verbose=0)
  *     for that number of RTC ticks before it finally being run again with
  *     verbose=verbose. Intended for recording testcase power measurements. 
  * @return Wether the testcase passed or failed (see TCPASS and TCFAIL)
  */
int tc_funcs_run_testcase(testcase_id_t tc, uint32_t verbose, uint64_t repeat_delay);

#endif // TC_FUNCTIONS_H
