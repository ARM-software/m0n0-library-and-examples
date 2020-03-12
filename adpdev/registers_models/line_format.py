#!/usr/bin/env python3
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

class Line_Format:
    def __init__(self, reg_name, data_start_col, data_pad):
        self._reg_name = str(reg_name).upper()
        if ' ' in self._reg_name:
            raise ValueError("Register name ("+self._reg_name+
                    ") contains a space")
        self._data_start_col = int(data_start_col)
        self._data_pad = str(int(data_pad))
        self._existing_names = {}
    
    def fmt(self, name, val, indent=0,reg=None,postfix=None):
        if name.upper().split('_')[0] == 'RESERVED':
            return ''
        name = 'R{:02d}_{}'.format(reg,name) if reg else name
        name = name+'_'+postfix if postfix else name
        if ' ' in name:
            raise ValueError("Reg name ("+name+
                    ") contains a space")
        prefix=self._reg_name+'_'
        defpre = '\n'+(' '*indent)+'#define '+prefix
        temp =  defpre+name.upper()
        if temp in self._existing_names:
            raise ValueError(temp+": already exists!")
        self._existing_names[temp] = 1
        pad = self._data_start_col - len(temp)
        pad = str(1 if pad < 1 else pad)
        temp += (' {: >'+pad+
                '}{:0'+self._data_pad+'X}').format(
                        '0x',
                        val)
        return temp

