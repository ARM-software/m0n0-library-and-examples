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

import json
import os
import logging

import line_format

_unused_string = 'RESERVED'

HTML_TAB = '&nbsp;&nbsp;'

class Registers_Model:
    def __init__(self, reg_config, logger=None, filter_internal=True):
        self._logger = logger or logging.getLogger(__name__)
        self._logger.debug('Creating register model with "{}"'.format(
                reg_config))
        self._reg_config_name = reg_config
        self.__regs = None
        self.__table_colour_head = 'ArmBlue'
        self.__table_colour_l1 = 'white'
        self.__table_colour_l2 = 'ArmLightGray'
        self.__table_colour_l3 = 'ArmDarkGray'
        self._base_address = None
        self._reg_offset = 1
        self._set_addr_offset = None
        self._clr_addr_offset = None
        self._read_driver = {}
        self._write_driver = {}
        self._read_driver['default'] = None
        self._write_driver['default'] = None
        if reg_config.lower().endswith('.yml') or \
                reg_config.lower().endswith('.yaml'):
            self._logger.debug("Parsing YAML...")
            validate = True
            import yaml
            validate = False
            #try:
            #    import jsonschema
            #except ModuleNotFoundError:
            #    validate= False 
            #schema = ""
            #with open('reg_model_schema.yaml', 'r') as f:
            #    schema = yaml.safe_load(f)
            #f.closed
            with open(reg_config, 'r') as f:
                temp = yaml.safe_load(f)
                if validate:
                    jsonschema.validate(temp,schema)
                self.__regs = temp['registers']
                self.__data_bits = temp['data_bits']
                if 'base_addr' in temp:
                    self._base_address = temp['base_addr']
                if 'reg_offset' in temp:
                    self._reg_offset = temp['reg_offset']
                if 'set_addr_offset' in temp:
                    self._set_addr_offset = temp['set_addr_offset']
                if 'clr_addr_offset' in temp:
                    self._clr_addr_offset = temp['clr_addr_offset']
            f.closed
        else:
            raise IOError(
                "reg_config ({}) has wrong extension".format(reg_config))
        # filter internal registers
        if filter_internal:
            self._filter_internal()
        # initialise to POR values
        for key in self.__regs:
            if 'por' in self.__regs[key]:
                self.__regs[key]['val'] = self.__regs[key]['por']
                self._logger.debug("Initialised 0x{:02X} to POR (0x{:02X})".format(
                    key, self.__regs[key]['por']))
            elif 'read_only' in self.__regs[key]:
                self.__regs[key]['por'] = 0
                self.__regs[key]['val'] = 0
                self._logger.debug("Initialised 0x{:02X} to POR (0x{:02X}) (READ ONLY)".format(
                    key, 0))
            else:
                raise ValueError("Register must have 'por' or 'read_only' key ({})"
                                 .format(self.__regs[key]))
            if not 'description' in self.__regs[key]:
                self.__regs[key]['description'] = ""

    def _filter_internal(self):
        temp_regs = {}
        for r in self.__regs:
            if 'internal' in self.__regs[r]:
                if self.__regs[r]['internal']:
                    # if there is 'internal' and it is true
                    continue
            # copy everything but bitgroups
            temp_reg = {}
            for prop in self.__regs[r]:
                if prop == 'bit_groups':
                    temp_bgs = []
                    for bg in self.__regs[r]['bit_groups']:
                        if 'internal' in bg:
                            if bg['internal']:
                                continue
                        temp_bgs.append(bg)
                    temp_reg['bit_groups'] = temp_bgs
                else:
                    temp_reg[prop] = self.__regs[r][prop]
            temp_regs[r] = temp_reg
        #print("REGS: ")
        #print(self.__regs)
        #print("Temp regs")
        #print(temp_regs)
        self.__regs = temp_regs
                    
    def set_read_driver(self, driver_func, name=None):
        name = name or 'default'
        self._read_driver[name] = driver_func

    def set_write_driver(self, driver_func, name=None):
        name = name or 'default'
        self._write_driver[name] = driver_func

    # Adds 'unused' to any non-specified bit groups
    # (only if there is already a bit group defined
    # for that register
    def add_unused_bits_and_order(self):
        for r in self.__regs:
            if 'bit_groups' in self.__regs[r]:
                self.check_bitgroup(self.__regs[r]['bit_groups'])
                used_bits = [-1 for x in range(0, self.__data_bits)]
                for i in range(0, len(self.__regs[r]['bit_groups'])):
                    bg = self.__regs[r]['bit_groups'][i]
                    if 'bit' in bg:
                        if bg['bit'] >= 0 and bg['bit'] < self.__data_bits:
                            used_bits[bg['bit']] = i
                        else:
                            raise ValueError(
                                "Processing {}, bit out of range".format(bg))
                    else:
                        # msb and lsb should be there
                        if (bg['msb'] >= 0 and bg['msb'] < self.__data_bits) and \
                                (bg['lsb'] >= 0 and bg['lsb'] < self.__data_bits):
                            if bg['lsb'] >= bg['msb']:
                                raise ValueError(
                                    "Processing {}, lsb>=msb".format(bg))
                            used_bits[bg['lsb']:bg['msb']+1] = [i] * \
                                (bg['msb']+1 - bg['lsb'])
                        else:
                            raise ValueError(
                                "Processing {}, msb/lsb out of range".format(bg))
                new_bitgroups = []
                current_lsb = 0
                # extends by one so last group is captured
                used_bits.append(-2)
                for i in range(1, len(used_bits)):
                    if used_bits[i] != used_bits[i-1]:
                        # transition
                        lsb = current_lsb
                        current_lsb = i
                        msb = i-1
                        if used_bits[i] == -3:
                            break
                        elif used_bits[i-1] == -1:  # unused
                            if msb == lsb:
                                new_bitgroups.append({
                                    'name': _unused_string,
                                    'bit': lsb,
                                    'description': ''
                                })
                            else:
                                new_bitgroups.append({
                                    'name': _unused_string,
                                    'lsb': lsb,
                                    'msb': msb,
                                    'description': ''
                                })
                        else:
                            new_bitgroups.append(
                                self.__regs[r]['bit_groups'][used_bits[i-1]])
                self.__regs[r]['bit_groups'] = new_bitgroups

    def check_bitgroup(self, bitgroup):
        for bg in bitgroup:
            if 'bit' in bg:
                if bg['bit'] >= 0 and bg['bit'] < self.__data_bits:
                    return
            elif 'msb' in bg and 'lsb' in bg:
                if bg['msb'] >= 0 and bg['msb'] < self.__data_bits \
                        and bg['lsb'] >= 0 and bg['lsb'] < self.__data_bits:
                    if bg['lsb'] < bg['msb']:
                        return
            else:
                raise ValueError("Error in bitgroup: {}".format(bitgroup))

    '''
  def break_ranges

  def get_next_lsb(bitgroups,after):
    cur_res = 9999999999999 # large number
    for bg in bitgroups:
      #if 
    
    
  def add_unused_bits_2(self):
    for r in self.__regs:
      print (self.__regs[r])
      if 'bit_groups' in self.__regs[r]:
        check_bitgroup(self.__regs[r]['bit_groups'])
        check_bitgroup_overlap(self.__regs[r]['bit_groups'])
        finished_groups = False
        find_lower = 0;
        while finished_groups == False:
          print("finding: {}".format(find_lower)) 
          res = None
          for bg in self.__regs[r]['bit_groups']:
            if 'bit' in bg:
              if bg['bit'] == find_lower:
                res = bg
                break
            if 'lsb' in bg:
              if bg['lsb'] == find_lower:
                res = bg
                break
          #if 
  '''

    def __str__(self):
        return self.get_reg_string()

    def as_markdown(self, expand_bitgroups=False):
        output_text = '| Address | Name            | Description              | POR |'
        output_text += '\n| ------- | --------------- | ------------------------ | --- |'
        hex_digits = int((self.__data_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        for k in self.__regs:
            output_text += ("\n|0x{:02X} | `{}` | {} |"+fmt+"|").format(
                k, self.__regs[k]['name'], self.__regs[k]['description'], self.__regs[k]['por'])
            if ('bit_groups' in self.__regs[k]) and expand_bitgroups:
                for b in self.__regs[k]['bit_groups']:
                    if 'bit' in b:
                        output_text += '\n|  |  | [' + \
                            str(b['bit'])+'] `'+b['name']+'` |'
                    else:
                        # print('['+str(b['msb'])+':'+str(b['lsb'])+']'+b['name'])
                        output_text += '\n|  |  | ['+str(b['msb'])+':'+str(
                            b['lsb'])+'] `'+b['name']+'` |'
        return output_text

    def reg_bits(self, val, msb, lsb):
        new_val = "{0:0100b}".format(val)
        new_val = new_val[::-1]
        new_val = new_val[lsb:msb+1]
        new_val = list(new_val)
        new_val = [new_val[i]+' ' if ((i+1) % 8 == 0) else new_val[i]
                   for i in range(0, len(new_val))]
        new_val = ''.join(new_val)
        new_val = new_val.strip()
        return (str(len(new_val.replace(' ','')))+'b'+new_val[::-1])

    # note that the returned value does not have the base address included
    def _get_highest_address(self):
        highest = 0
        for k in self.__regs:
            if k > highest:
                highest = k
        return (k*self._reg_offset)

    def as_code_constants(self,reg_name):
        hex_digits = int((self.__data_bits / 4)+0.5)
        lf = line_format.Line_Format(reg_name,60,hex_digits)
        output_text = ''
        output_text += '\n\n// '+reg_name
        output_text += '\n// Generated from: '+self._reg_config_name+'\n'
        base = 0 if not  self._base_address else self._base_address
        output_text += lf.fmt('BASE_ADDR', base)
        output_text += lf.fmt('SIZE', self._get_highest_address())
        if self._set_addr_offset:
            output_text += lf.fmt("SET_OFFSET", self._set_addr_offset)
        if self._clr_addr_offset:
            output_text += lf.fmt("CLR_OFFSET", self._clr_addr_offset)
        for k in self.__regs:
            cur_reg = self.__regs[k]
            output_text += lf.fmt(
                cur_reg['name'],
                base + (k*self._reg_offset),
                postfix='REG')
            if ('bit_groups' in self.__regs[k]):
                for b in self.__regs[k]['bit_groups']:
                    if 'bit' in b:
                        output_text += lf.fmt(
                            b['name'],
                            b['bit'],indent=2,reg=k,postfix='BIT_SHIFT')
                        output_text += lf.fmt(
                            b['name'],
                            1<<b['bit'],indent=2,reg=k,postfix='BIT_MASK')
                    else:
                        mask = 0
                        for t in range(b['lsb'],b['msb']+1):
                            mask |= 1 << t
                        output_text += lf.fmt(
                            b['name'],
                            b['lsb'],indent=2,reg=k,postfix='BIT_SHIFT')
                        output_text += lf.fmt(
                            b['name'],
                            mask,indent=2,reg=k,postfix='BIT_MASK')
        return output_text

    def as_latex(self, content_only=True,
                 detail_level=0,
                 reg_address=None,
                 show_offset_only=True,
                 caption="caption", label="label"):
        output_text = ''
        temp_base_addr = self._base_address
        if show_offset_only:
            temp_base_addr = 0
        if not content_only:
            output_text += '\n'+r'\begin{table}[t]'
            output_text += '\n'+r'\centering'
            output_text += '\n'+r'\caption{}'
            output_text += '\n'+r'\label{tab:my-table}'
        #output_text += '\n'+r'\begin{tabular}{|l|l|p{6.5cm}|l|}'
        #output_text += '\n'+r'\begin{longtable}{|l|l|p{6.0cm}|p{2cm}|}'
        #output_text += '\n'+r'\centering'
        hex_digits = int((self.__data_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        output_text += r'\endfirsthead'
        output_text += '\n'+r'\endhead'
        output_text += '\n'+r'\hline'
        #base_addr_string =""
        #if self._base_address: # not used
        #    base_addr_string = r'\tablefootnote{'+"0x{:02X}".format(self._base_address)+r'}'
        #    base_addr_string = r'\tablefootnote{banana}'
        base_addr_string = ""
        if (not reg_address) and self._base_address and show_offset_only:
            base_addr_string = r" \\(+0x{:02X})".format(self._base_address)
        output_text += '\n'+r'\rowcolor{'+self.__table_colour_head \
            + r'}\shortstack[l]{Address '+base_addr_string+r'} & Name & Description  & POR  \\ \hline'
        is_first = True
        for k in self.__regs:
            if reg_address:
                if reg_address != k:
                    continue 
            cur_reg = self.__regs[k]
            output_text += ("\n0x{:02X} & {} & {} & "+fmt+r" \\ \hline").format(
                    k*self._reg_offset if not temp_base_addr else (k*self._reg_offset)+self._base_address,
                    self.__regs[k]['name'].replace('_', r'\_'),
                    self.__regs[k]['description'].replace('_', r'\_'),
                    self.__regs[k]['por'])
            if ('bit_groups' in self.__regs[k]) and detail_level > 0:
                for b in self.__regs[k]['bit_groups']:
                    output_text += '\n'
                    temp_description = b['description'].replace('_', r'\_') if \
                        'description' in b else ' '
                    if 'bit' in b:
                        bg_por = '-' if 'read_only' in cur_reg else '{\\footnotesize '+self.reg_bits(
                            cur_reg['por'], b['bit'], b['bit'])+'}' 
                        output_text += r'\relax' + '\n & '+r'\cellcolor{'+self.__table_colour_l2+'}'+'['+str(b['bit'])+']  ' \
                            + b['name'].replace('_', r'\_')+' & '+r'\cellcolor{'+self.__table_colour_l2+'}' \
                            + temp_description + ' & '+r'\cellcolor{'+self.__table_colour_l2+'}' \
                            + bg_por
                    else:
                        bg_por = '-' if 'read_only' in cur_reg else '{\\footnotesize '+self.reg_bits(
                            cur_reg['por'], b['msb'], b['lsb'])+'}'
                        output_text += r'\relax' + '\n & '+r'\cellcolor{'+self.__table_colour_l2+'}'+' ['+str(b['msb'])+':'+str(b['lsb'])+']  ' \
                            + b['name'].replace('_', r'\_')+' & '+r'\cellcolor{'+self.__table_colour_l2+'}' \
                            + temp_description+' & '+r'\cellcolor{'+self.__table_colour_l2+'}' \
                            + bg_por
                    output_text += r' \\ \hline'
                    if 'value_table' in b and detail_level > 1:
                        for v in b['value_table']:
                            output_text += '\n' 
                            output_text += r'   & & '+r' \cellcolor{' + self.__table_colour_l3+'} '
                            output_text += str(v)+': ' + \
                                str(b['value_table'][v]).replace('_', r'\_')
                            output_text += r' &   \\ \hline'

        #output_text += '\n'+r'\end{tabular}'
        #output_text += '\n'+r'\end{longtable}'
        if not content_only:
            output_text += '\n'+r'\end{table}'
        return output_text

    def as_html(self,
                 detail_level=0,
                 show_offset_only=True,
                 show_notes=False):
        output_text = ''
        temp_base_addr = self._base_address
        if show_offset_only:
            temp_base_addr = 0
        output_text += '\n<p>Base: 0x{:02X}</p><p>Data bits: {}</p>'.format(
                self._base_address if self._base_address else 0,
                self.__data_bits)
        output_text += '<p>Reg offset: {}</p>'.format(
                self._reg_offset)
        if self._set_addr_offset:
            output_text += '<p>Set offset: 0x{:02X}</p><p>Clear offset: 0x{:02X}</p>'.format(
                    self._set_addr_offset,
                    self._clr_addr_offset)
        output_text += '\n<table>\n'
        output_text += '<tr>\n'
        output_text += '    <th>Address</th>\n'
        output_text += '    <th>Name</th>\n'
        output_text += '    <th>Description</th>\n'
        output_text += '    <th>POR</th>\n'
        output_text += '</tr>'
        hex_digits = int((self.__data_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        for k in self.__regs:
            cur_reg = self.__regs[k]
            if 'internal' not in self.__regs[k]:
                output_text += '\n\n\n<tr class="reg">'
            else:
                if self.__regs[k]['internal']:
                    output_text += '\n\n\n<tr class="internal-reg">'
                else:
                    output_text += '\n\n\n<tr class="reg">'
            output_text += ("\n<td>0x{:02X}</td> <td>{} </td><td> {} </td><td> "+fmt+"</td>").format(
                    k*self._reg_offset if not temp_base_addr \
                            else (k*self._reg_offset)+self._base_address,
                    self.__regs[k]['name'],
                    self.__regs[k]['description'] if not show_notes else \
                            "<p>{}:</p>\n{}".format(
                                    self.__regs[k]['description'],
                                    "" if 'notes' not in self.__regs[k] \
                                            else "<p>"+("</p><p>".join(self.__regs[k]['notes'].split('\n'))).replace('\\t',HTML_TAB)+"</p>" ),
                    self.__regs[k]['por'])
            output_text += '\n</tr>\n'
            if ('bit_groups' in self.__regs[k]) and detail_level > 0:
                for b in self.__regs[k]['bit_groups']:
                    bg_class = "bg"
                    if 'internal' not in b:
                        bg_class = "bg"
                    else:
                        if b['internal']:
                            bg_class = "internal-bg"
                        else:
                            bg_class = "bg"
                    output_text += '\n<tr >'
                    temp_description = b['description'] if \
                        'description' in b else ' '
                    if show_notes and 'notes' in b:
                        temp_description += ':\n'+"<p>"+("</p><p>".join(b['notes'].split('\n'))).replace('\\t',HTML_TAB)+"</p>" 
                    if 'bit' in b:
                        bg_por = '-' if 'read_only' in cur_reg else ''+self.reg_bits(
                            cur_reg['por'], b['bit'], b['bit'])+''
                        output_text += '\n <td></td><td class="'+bg_class+'"> ['+str(b['bit'])+']  ' \
                            + b['name']+' </td><td class="'+bg_class+'"> ' \
                            + temp_description + ' </td><td class="'+bg_class+'"> ' \
                            + bg_por + '</td>'
                    else:
                        bg_por = '-' if 'read_only' in cur_reg else ''+self.reg_bits(
                            cur_reg['por'], b['msb'], b['lsb'])+''
                        output_text += '\n <td></td><td class="'+bg_class+'"> ['+str(b['msb'])+':'+str(b['lsb'])+']  ' \
                            + b['name']+' </td><td class="'+bg_class+'"> ' \
                            + temp_description+' </td><td class="'+bg_class+'"> ' \
                            + bg_por + '</td>'
                    output_text += '\n</tr>'
                    if 'value_table' in b and detail_level > 1:
                        for v in b['value_table']:
                            output_text += '\n<tr>'
                            output_text += '\n   <td></td><td></td>'
                            output_text += '<td class="ltable">'+str(v)+': ' + \
                                str(b['value_table'][v]) + '</td>'
                            output_text += ' <td></td>\n'
                            output_text += '\n</tr>'
        output_text += '\n</table>'
        return output_text



    def get_value_table(self,addr,bit_group):
        return self.read_model(addr,bit_group=bit_group)['value_table']

    def get_base_address(self):
      return self._base_address

    def get_reg_string(self, data_base='hex'):
        result_str = ""
        if (self._base_address):
            result_str += "base address: 0x{:X}\n".format(self._base_address)
        hex_digits = int((self.__data_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        if data_base == 'bin':
            fmt = 'b{:0'+str(self.__data_bits)+'b}'
        elif data_base == 'dec':
            fmt = '{:0'+str(len(str(2**self.__data_bits)))+'d}'
        return result_str +'\n'.join([("0x{:02X}: "+fmt+"    POR: "+fmt+"    {}").format(
            x, self.__regs[x]['val'], self.__regs[x]['por'], self.__regs[x]['name'])
            for x in sorted([k for k in self.__regs])])

    def read_model(self, addr, bit_group=None):
        num_addr = None
        if isinstance(addr, (int)):
            if not addr in self.__regs:
                raise ValueError(
                    "Error: address (0x{:02X}) not valid!".format(addr))
            res = self.__regs[addr]
            num_addr = addr
        elif isinstance(addr, str):
            key = None
            val = None
            for k, v in self.__regs.items():
                if v['name'] == addr:
                    key = k
                    val = v
            if key is None:
                raise ValueError("{} not in self.__regs".format(addr))
            num_addr = key
            res = val
            # if not addr in [x['name'] for x in self.__regs]: # TODO Check
            #  raise ValueError("Reg model does not contain: {}".format(addr))
            #res = [x for x in self.__regs if x['name'] == addr]
            # if len(res) != 1:
            #  raise ValueError("length of res != 1, res: {}".format(res))
            #res = res[0]
        else:
            self._logger.error("Invalid address")
            raise ValueError("Error: address ({}) is not valid".format(addr))
        read_only = True if 'read_only' in res else False
        if not bit_group:
            return {'val': res['val'], 'msb': None, 'lsb': None, 'addr': num_addr, 'full_reg': res['val'], "read_only" : read_only, 'value_table': None}
        bg = [x for x in res['bit_groups'] if x['name'] == bit_group]
        if len(bg) != 1:
            print(res['bit_groups'])
            print(bg)
            raise ValueError("len(bg) != 1, {}".format(bg))
        bg = bg[0]
        result = None
        msb = None
        lsb = None
        if 'bit' in bg:
            msb = bg['bit']
            lsb = bg['bit']
        else:
            msb = bg['msb']
            lsb = bg['lsb']
        val_table = None
        if 'value_table' in bg:
            val_table = bg['value_table']
        result = self.select_bit_range(res['val'], msb, lsb)
        return {'val': result, 'msb': msb, 'lsb': lsb, 'addr': num_addr, 'full_reg': res['val'], "read_only" : read_only, 'value_table' : val_table}

    def read_device(self, address, driver_name='default'):
        self._logger.debug("Read device address: {:0X} using driver: {}".format(
            address,
            driver_name))
        return self._read_driver[driver_name](address)

    def write_device(self, address, data, driver_name='default'):
        self._logger.debug("Write device address: {:0X}, data: {:0X}, using driver: {}".format(
            address,
            data,
            driver_name))
        return self._write_driver[driver_name](address, data)

    def select_bit_range(self, val, msb, lsb):
        if val < 0:
            raise ValueError("Value must be positive")
        self._logger.debug('Select bit range in: {0:b}'.format(val))
        val_bin_r = (bin(val)[2:])[::-1]  # reverse
        val_bin_r += ''.join(['0']*1000)  # pad
        new_val = (val_bin_r[lsb:msb+1])[::-1]  # reverse
        assert len(new_val) == (msb-lsb)+1
        new_val = int(new_val, 2)
        self._logger.debug('Select bit range out: {0:b}'.format(new_val))
        return new_val

    def write_set(self,
              addr,
              data,
              write_driver_name='default'):
        self._logger.debug("Write set: A: {}, Data {:X}".format(addr,data))
        if not self._set_addr_offset:
            raise NotImplementedError("This register does not have set function")
        model_rd_data = self.read_model(addr)
        full_reg_val = model_rd_data['full_reg']
        if data > 2**self.__data_bits - 1 or data < 0:
            raise ValueError("Error: data larger than data_bits bits!")
        new_val = data
        if isinstance(addr, (int)):
            self.__regs[addr]['val'] = model_rd_data['val'] | data
            self.write_device(
                self.get_device_address(addr) + self._set_addr_offset, 
                new_val,
                driver_name=write_driver_name)
        else:  # str
            self.__regs[model_rd_data['addr']]['val']  = model_rd_data['val'] | data
            self.write_device(
                self.get_device_address(model_rd_data['addr']) + self._set_addr_offset,
                new_val,
                driver_name=write_driver_name)

    def write_clear(self,
              addr,
              data,
              write_driver_name='default'):
        if not self._clr_addr_offset:
            raise NotImplementedError("This register does not have clear function")
        new_val = data
        model_rd_data = self.read_model(addr)
        full_reg_val = model_rd_data['full_reg']
        if data > 2**self.__data_bits - 1 or data < 0:
            raise ValueError("Error: data larger than data_bits bits!")
        if isinstance(addr, (int)):
            self.__regs[addr]['val'] = model_rd_data['val'] & (~data)
            self.write_device(
                self.get_device_address(addr) + self._clr_addr_offset, 
                new_val,
                driver_name=write_driver_name)
        else:  # str
            self.__regs[model_rd_data['addr']]['val'] = model_rd_data['val'] & (~data)
            self.write_device(
                self.get_device_address(model_rd_data['addr']) + self._clr_addr_offset,
                new_val,
                driver_name=write_driver_name)

    def write(self,
              addr,
              data,
              bit_group=None,
              write_driver_name='default',
              read_device=False,
              read_driver_name='default'):
        model_rd_data = self.read_model(addr, bit_group)
        hw_read_val = None
        if read_device:
            hw_read_val = self.read_device(self.get_device_address(model_rd_data['addr']), read_driver_name)
        full_reg_val = model_rd_data['full_reg']
        if hw_read_val is not None:
            full_reg_val = hw_read_val
        if bit_group:
            # it is a bit gtroup
            if data < 0:
                raise ValueError("Data must be positive")
            msb = model_rd_data['msb']
            lsb = model_rd_data['lsb']
            bg_len = (msb-lsb)+1
            bin_data = (bin(data)[2:])[::-1]  # reverse
            if len(bin_data) > bg_len:
                raise ValueError("Data larger than bit_group bits can hold")
            bin_data += ''.join(['0']*1000)  # pad
            bin_full_reg = (bin(full_reg_val)[2:])[::-1]
            bin_full_reg += ''.join(['0']*1000)  # pad
            bin_data = list(bin_data)  # conver to list
            bin_full_reg = list(bin_full_reg)  # convert to list
            bin_full_reg[lsb:msb+1] = bin_data[0:bg_len]
            bin_full_reg = ''.join(bin_full_reg)  # convert back to string
            bin_full_reg = bin_full_reg[::-1]
            new_val = int(bin_full_reg, 2)
            if isinstance(addr, (int)):
                self.__regs[addr]['val'] = new_val
                self.write_device(
                    self.get_device_address(addr), 
                    new_val,
                    driver_name=write_driver_name)
            else:  # str
                self.__regs[model_rd_data['addr']]['val'] = new_val
                self.write_device(
                    self.get_device_address(model_rd_data['addr']),
                    new_val,
                    driver_name=write_driver_name)
        else:
            # a whole register value
            new_val = data
            if data > 2**self.__data_bits - 1 or data < 0:
                raise ValueError("Error: data larger than data_bits bits!")
            if isinstance(addr, (int)):
                if not addr in self.__regs:
                    raise ValueError(
                        "Error: address (0x{:02X}) not valid!".format(addr))
                self.__regs[addr]['val'] = data
                self.write_device(
                    self.get_device_address(addr), 
                    new_val,
                    driver_name=write_driver_name)
            else:  # str
                self.__regs[model_rd_data['addr']]['val'] = data
                self.write_device(
                    self.get_device_address(model_rd_data['addr']),
                    new_val,
                    driver_name=write_driver_name)
        return True

    def read(self,
             addr,
             bit_group=None,
             driver_name='default'):
        model_data = self.read_model(addr, bit_group)
        if not self._read_driver[driver_name]:
            self._logger.warning("No read driver, returning modelled value")
            return model_data['val']
        # with hardware read driver
        # if addr is a string, need the model to provide the numerical addr
        if isinstance(addr, (int)):
            numerical_addr = addr
        else:
            numerical_addr = model_data['addr']
        hw_val = self.read_device(self.get_device_address(numerical_addr), driver_name)
        if hw_val != model_data['full_reg'] and not model_data['read_only']:
            self._logger.warning("READ MISMATCH {}: Addr: 0x{:X} (0x{:X}) device: 0x{:X}, model: 0x{:X}".format(
                self._reg_config_name,
                numerical_addr,
                self.get_device_address(numerical_addr),
                hw_val,
                model_data['val']
            ))
        if not bit_group:
            return hw_val
        else:
            return self.select_bit_range(hw_val, model_data['msb'], model_data['lsb'])

    def get_valid_addresses(self):
        return list(self.__regs.keys())

    def get_map_address(self,address):
        if isinstance(address, (int)):
            return self.get_device_address(address)
        else:
            read_res = self.read_model(address)
            return self.get_device_address(read_res['addr'])

    def get_device_address(self,address):
        if self._base_address:
            address = (address*self._reg_offset) + self._base_address
            self._logger.debug("Adding base ({:0X}) and offsef ({:0X}). " \
                +" New address: {:0X}".format(
                    self._base_address,
                    self._reg_offset,
                    address))
        return address

    def compare_reg(self,address):
        model_res = self.read_model(address)
        val  = model_res['val']
        return("A: {} -- HW: {:X}, model: {:X}".format(
                address, self.read(address),val))

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(
        description='Register Model')
    parser.add_argument('-c', '--register-config', required=True,
                        help="Register configuration file")
    args = parser.parse_args()
    my_reg = Registers_Model(args.register_config)
    print(my_reg)
    print("Now printing markdown")
    with open('temp.md', 'w') as f:
        f.write(my_reg.as_markdown(detail_level=2))
    f.closed
    with open('temp.tex', 'w') as f:
        f.write(my_reg.as_latex(detail_level=2))
    f.closed
    my_reg.add_unused_bits_and_order()
    with open('temp-after-reorder.tex', 'w') as f:
        f.write(my_reg.as_latex(detail_level=2))
    f.closed
    print(my_reg.register_detail_as_latex(0x8))

    '''
  my_pcsm.write(2,23)
  print(my_pcsm.read(2))
  print("________________________________")
  print(my_pcsm.get_reg_string())
  print(my_pcsm.get_reg_string(data_base='dec'))
  print(my_pcsm.get_reg_string(data_base='bin'))
  print(my_pcsm)
  '''
