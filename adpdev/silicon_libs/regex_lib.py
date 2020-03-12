#!/usr/bin/env python
################################################################################
# Copyright (c) 2020, Arm Limited
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
################################################################################
import re

'''
Usage example:
python library/regex_lib.py -i dump.txt -f rom_count_classifications
'''

'''
If possible try to stick with a consistent return dictionary of any captured
data. E.g. always return a dictionary, even if no match (this is for automatically
creating data table with correct headers). Also always include a match boolean
output, so that a fail can be differentiated between a reported fail and a
match fail
'''

#M0N0S2_romcheck_expre = r'CPUID is: 0x(.*)\sDIAG_VBAT is: 0x([0-9a-f]*)\sDEVE: (\d)\srtc 0x(.*)\srtc start time 0x([0-9a-f]*)[\s]*Waiting for ADP direction...[\s]*Ecaping waitforadp[\s]*\*\*Running KWS\*\*[\s]*\*\* STARTING KWS \*\*[\s]*C-([\d])*, ([\d]*)\s'

M0N0S2_kws_testcase_w_error = r'--- TCID: (\d+) ---[\s]*\** STARTING KWS \*\*[\s]*(^RC|^C-([\d]+),) ([\S]+)[ ]*$'
M0N0S2_kws_romcheck_w_error = r'CPUID is: 0x(.*)\sDIAG_VBAT is: 0x([0-9a-f]*)\sDEVE: (\d)\srtc 0x(.*)\srtc start time 0x([0-9a-f]*)[\s]*Waiting for ADP direction...[\s]*Ecaping waitforadp[\s]*\*\*Running KWS\*\*[\s]*\*\* STARTING KWS \*\*[\s]*(^RC|^C-([\d]+),) ([\S]+)[ ]*$'
M0N0S2_wait_for_adp = r'Waiting for ADP direction..'


def M0N0S2_rom_check(string_input,logger):
  res = re.search(
    M0N0S2_kws_romcheck_w_error,
    string_input,
    re.MULTILINE) 
  if res:
    if logger:
      logger.debug('cpuid: '+res.group(1))
      logger.debug('diagvbat: '+res.group(2))
      logger.debug('deve'+res.group(3))
      logger.debug('rtc: 0x'+res.group(4))
      logger.debug('rtc: 0x'+res.group(5))
      logger.debug('classification: '+res.group(6))
    kws_error_message = False
    if res.group(8).strip() == 'error':
        classification = None
        probability = None
        kws_error_message = True
        logger.info("KWS Error (low perf?)")
    else:
        classification = res.group(7)
        probability = res.group(8)
        logger.info("KWS Passed")
    return {
      'match' : True,
      'cpuid' : '0x'+res.group(1),
      'diagvbat' : '0x'+res.group(2),
      'deve' : res.group(3),
      'rtc' : '0x'+res.group(4),
      'rtc_start_time' : '0x'+res.group(5),
      'classification' : classification,
      'probability' : probability,
      'kws_error_message' : kws_error_message
    }
  else:
    logger.info("RESULT: NO MATCH")
    return {
      'match' : False,
      'cpuid' : None,
      'diagvbat' : None,
      'deve' : None,
      'rtc' : None,
      'rtc_start_time' : None,
      'classification' : None,
      'probability' : None,
      'kws_error_message' : None
    }


# NOTE: S1, not S2
def rom_info_regex(string_input,verbose=True):
  #res =re.match(r'CPUID is: (.*)\sDIAG_VBAT is: (\d*)\sDEVE: (\d)\srtc 0x(.*)\s([\s\S]*?)- (\d) \*\*([\s\S]*?)(C-1.*)\v\v(C-1.*)', string_input, re.MULTILINE) 
  res = re.search(
    #r'CPUID is: (.*)\sDIAG_VBAT is: (\d*)\sDEVE: (\d)\srtc 0x(.*)\s([\s\S]*?)- (\d) \*\*([\s\S]*?)(C-1.*)\s\s(C-1.*)',
    #r'CPUID is: (.*)\sDIAG_VBAT is: (\d*)\sDEVE: (\d)\srtc 0x(.*)\s([\s\S]*?)- (\d) \*\*([\s\S]*?)(C-1, 12)',
    r'CPUID is: (.*)\sDIAG_VBAT is: ([0-9a-f]*)\sDEVE: (\d)\srtc 0x(.*)\s([\s\S]*?)- (\d) \*\*([\s\S]*?)(C-1, 12)',
    string_input,
    re.MULTILINE) 
  if res:
    return {
      'match' : True,
      'cpuid' : res.group(1),
      'diagvbat' : res.group(2),
      'deve' : res.group(3),
      'rtc' : res.group(4),
      'pass' : res.group(6),
      'classification' : res.group(8)
    }
  else:
    print("RESULT: NO MATCH")
    if verbose:
      print("No match!")
    return {
      'match' : False,
      'cpuid' : -1,
      'diagvbat' : -1,
      'deve' : -1,
      'rtc' : -1,
      'pass' : False
  }

def kws_testcase_decode(string_input,logger):
    res = re.search(
            M0N0S2_kws_testcase_w_error,
            string_input,
            re.MULTILINE)
    if res:
        logger.debug("Testcase ID: "+res.group(1))
        logger.debug("TC printf body:"+res.group(0))
        to_ret = {}
        to_ret['match'] = True
        to_ret['testcase_id'] = res.group(1)
        to_ret['body_printf'] = res.group(0)
        if res.group(4) == "error":
            to_ret['classification'] = None
            to_ret['probability'] = None
            to_ret['kws_error_message'] = True
            logger.warning("KWS Error Message (perf too low?)")
        else:
            to_ret['classification'] = res.group(3)
            to_ret['probability'] = res.group(4)
            to_ret['kws_error_message'] = False
            logger.warning("KWS Passed")
        return to_ret
    else:
        logger.error("NO OUTPUT MATCH")
        return {
          'match' : False,
          'testcase_id' : -1,
          'body_printf' : '',
        }




def testcase_result_decode(string_input,logger):
  res = re.search(
    r'--- TCID: (\d*) ---\s([\s\S]*)^TC STATUS:(.+)$',
    string_input,
    re.MULTILINE) 
  if res:
    logger.debug("Testcase ID: "+res.group(1))
    logger.debug("TC printf body:"+res.group(2))
    logger.debug("Pass fail: "+res.group(3))
    has_passed = False
    if res.group(3) == 'TCPASS':
      has_passed = True 
      logger.info("Testcase: PASS")
    elif res.group(3) == 'TCFAIL':
      has_passed = False
      logger.error("Testcase: FAIL")
    else: 
      logger.error("Testcase: MATCH, COULD NOT EXTRACT TCPASS/TCFAIL")
      return {
      'match' : False,
      'testcase_id' : -1,
      'body_printf' : '',
      'raw_passfail' : '',
      'has_passed' : False
      }
    return {
      'match' : True,
      'testcase_id' : int(res.group(1)),
      'body_printf' : res.group(2),
      'raw_passfail' : res.group(3),
      'has_passed' : has_passed
    }
  else:
    logger.error("Testcase: NO OUTPUT MATCH")
    return {
      'match' : False,
      'testcase_id' : -1,
      'body_printf' : '',
      'raw_passfail' : -1,
      'has_passed' : False
    }

 
def rom_count_classifications(string_input):
  res = re.findall(r'^C-1, 12$',string_input,re.MULTILINE)
  print("Number of classifications: {:02d}".format(len(res)))
  return len(res)

if __name__ == "__main__":
  import argparse
  parser = argparse.ArgumentParser(
        description='For testing the regex on saved output')
  parser.add_argument('-i','--input-dump',required=True,
        help="This is where you provided the saved text to apply")
  parser.add_argument('-f','--regex-func-name',required=True,
        help="The function name")
  parser.add_argument('-s','--strip-angle-brackets',required=False,
        action="store_true",
        help="If specified, this will strip any '<','>' in the input file")
  args = parser.parse_args()
  temp_raw = ''
  with open(args.input_dump,'r') as f:
    temp_raw = f.read() 
  f.closed
  if args.strip_angle_brackets:
    temp_raw = temp_raw.replace('<','').replace('>','')
  print("Input:")
  print(temp_raw)
  print("Running the function: {}".format(args.regex_func_name))
  result = locals()[args.regex_func_name](temp_raw)
  print("Finished")
  print("Printing result:")
  print(result)
