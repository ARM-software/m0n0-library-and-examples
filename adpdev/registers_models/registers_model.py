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
    """Loads register definitions from a YAML file, allows read/write interface with a model of the register, and allows register data to be viewed or exported. Read and write drivers can be set to enable the register models to be used to read/write to a real device over any interface. 
    
    :param reg_config: Path to the register definition file in YAML format
    :type reg_config: str
    :param logger: The logger object for logging messages to the console
                   and file
    :type logger: logging.Logger object
    :param filter_internal: If true (default), it removes all registers or bit
                            groups marked as internal (as they may cause
                            damage to a device if used incorrectly). 
    :type filter_internal: bool, optional
    """
    def __init__(self, reg_config, logger=None, filter_internal=True):
        """Constructor
        """
        self._logger = logger or logging.getLogger(__name__)
        self._logger.debug('Creating register model with "{}"'.format(
                reg_config))
        self._reg_config_name = reg_config
        self._regs = None
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
            with open(reg_config, 'r') as f:
                temp = yaml.safe_load(f)
                if validate:
                    jsonschema.validate(temp,schema)
                self._regs = temp['registers']
                self._data_bits = temp['data_bits']
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
        for key in self._regs:
            if 'por' in self._regs[key]:
                self._regs[key]['val'] = self._regs[key]['por']
                self._logger.debug("Initialised 0x{:02X} to POR (0x{:02X})".format(
                    key, self._regs[key]['por']))
            elif 'read_only' in self._regs[key]:
                self._regs[key]['por'] = 0
                self._regs[key]['val'] = 0
                self._logger.debug("Initialised 0x{:02X} to POR (0x{:02X}) (READ ONLY)".format(
                    key, 0))
            else:
                raise ValueError("Register must have 'por' or 'read_only' key ({})"
                                 .format(self._regs[key]))
            if not 'description' in self._regs[key]:
                self._regs[key]['description'] = ""

    def _filter_internal(self):
        """Omits all registers and/or bit groups marked as internal in the register definition from the model
        """
        temp_regs = {}
        for r in self._regs:
            if 'internal' in self._regs[r]:
                if self._regs[r]['internal']:
                    # if there is 'internal' and it is true
                    continue
            # copy everything but bitgroups
            temp_reg = {}
            for prop in self._regs[r]:
                if prop == 'bit_groups':
                    temp_bgs = []
                    for bg in self._regs[r]['bit_groups']:
                        if 'internal' in bg:
                            if bg['internal']:
                                continue
                        temp_bgs.append(bg)
                    temp_reg['bit_groups'] = temp_bgs
                else:
                    temp_reg[prop] = self._regs[r][prop]
            temp_regs[r] = temp_reg
        self._regs = temp_regs
                    
    def set_read_driver(self, driver_func, name=None):
        """Sets a read driver to the model. Typically used to make reads from the model read from a device over an interface.

        :param driver_func: The driver function (callback) used to read from the interface
        :type driver_func: function
        :param name: Name of the driver. If not specified then it sets the default read driver.
        :type name: str, optional
        """
        name = name or 'default'
        self._read_driver[name] = driver_func

    def set_write_driver(self, driver_func, name=None):
        """Sets a write driver to the model. Typically used to make writes to the model write to a device over an interface.

        :param driver_func: The driver function (callback) used to write to the interface
        :type driver_func: function
        :param name: Name of the driver. If not specified then it sets the default write driver.
        :type name: str, optional
        """
        name = name or 'default'
        self._write_driver[name] = driver_func

    def add_unused_bits_and_order(self):
        """Adds 'unused' to any non-specified bit groups (only if there is already a bit group defined for that register)
        """
        for r in self._regs:
            if 'bit_groups' in self._regs[r]:
                self.check_bitgroup(self._regs[r]['bit_groups'])
                used_bits = [-1 for x in range(0, self._data_bits)]
                for i in range(0, len(self._regs[r]['bit_groups'])):
                    bg = self._regs[r]['bit_groups'][i]
                    if 'bit' in bg:
                        if bg['bit'] >= 0 and bg['bit'] < self._data_bits:
                            used_bits[bg['bit']] = i
                        else:
                            raise ValueError(
                                "Processing {}, bit out of range".format(bg))
                    else:
                        # msb and lsb should be there
                        if (bg['msb'] >= 0 and bg['msb'] < self._data_bits) and \
                                (bg['lsb'] >= 0 and bg['lsb'] < self._data_bits):
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
                                self._regs[r]['bit_groups'][used_bits[i-1]])
                self._regs[r]['bit_groups'] = new_bitgroups

    def check_bitgroup(self, bitgroup):
        """Checks correctness of bit groups specified in definition file

        :param bitgroup: The single bit groups dict of a register 
                         (containing the individual bit groups)
        :type bitgroup: dict
        """
        for bg in bitgroup:
            if 'bit' in bg:
                if bg['bit'] >= 0 and bg['bit'] < self._data_bits:
                    return
            elif 'msb' in bg and 'lsb' in bg:
                if bg['msb'] >= 0 and bg['msb'] < self._data_bits \
                        and bg['lsb'] >= 0 and bg['lsb'] < self._data_bits:
                    if bg['lsb'] < bg['msb']:
                        return
            else:
                raise ValueError("Error in bitgroup: {}".format(bitgroup))

    def __str__(self):
        """Returns string representation of a register model
        """
        return self.get_reg_string()

    def as_markdown(self, expand_bitgroups=False):
        """Exports the register model as a markdown table

        :param expand_bitgroups: Whether to have a row for each individual
                                 bitgroup or not
        :type expand_bitgroups: bool, optional
        """
        output_text = '| Address | Name            | Description              | POR |'
        output_text += '\n| ------- | --------------- | ------------------------ | --- |'
        hex_digits = int((self._data_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        for k in self._regs:
            output_text += ("\n|0x{:02X} | `{}` | {} |"+fmt+"|").format(
                k, self._regs[k]['name'], self._regs[k]['description'], self._regs[k]['por'])
            if ('bit_groups' in self._regs[k]) and expand_bitgroups:
                for b in self._regs[k]['bit_groups']:
                    if 'bit' in b:
                        output_text += '\n|  |  | [' + \
                            str(b['bit'])+'] `'+b['name']+'` |'
                    else:
                        output_text += '\n|  |  | ['+str(b['msb'])+':'+str(
                            b['lsb'])+'] `'+b['name']+'` |'
        return output_text

    def reg_bits(self, val, msb, lsb):
        """Returns a string representation of a bitgroup in a binary string form (i.e. a register). E.g. used for bit group PoR binary values in latex and html documentation tables.

        :param val: the full register value (typically the PoR value)
        :type val: int
        :param msb: The MSB index of the bit group within the register
        :type msb: int
        :param lsb: The LSB index of the bit group within the register
        :type lsb: int
        :return: the binary bit group value
        :rtype: str
        """
        new_val = "{0:0100b}".format(val)
        new_val = new_val[::-1]
        new_val = new_val[lsb:msb+1]
        new_val = list(new_val)
        new_val = [new_val[i]+' ' if ((i+1) % 8 == 0) else new_val[i]
                   for i in range(0, len(new_val))]
        new_val = ''.join(new_val)
        new_val = new_val.strip()
        return (str(len(new_val.replace(' ','')))+'b'+new_val[::-1])

    def _get_highest_address(self):
        """Returns the highest address specified in the register (note that the returned value does not have the base address included). 
        
        :return: The highest register address
        :rtype: int
        """
        highest = 0
        for k in self._regs:
            if k > highest:
                highest = k
        return (k*self._reg_offset)

    def as_code_constants(self, reg_name):
        """Exports the register model information as code constants (defines) for generating the embedded code constants (register addresses and bit group masks

        :param reg_name: The name of the overall register file in the software
                         defines
        :type reg_name: str
        :return: The text of the defines and masks for the whole register file
        :rtype: str
        """
        hex_digits = int((self._data_bits / 4)+0.5)
        lf = line_format.Line_Format(reg_name, 60, hex_digits)
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
        for k in self._regs:
            cur_reg = self._regs[k]
            output_text += lf.fmt(
                cur_reg['name'],
                base + (k*self._reg_offset),
                postfix='REG')
            if ('bit_groups' in self._regs[k]):
                for b in self._regs[k]['bit_groups']:
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

    def as_latex(
                 self,
                 detail_level=0,
                 reg_address=None,
                 show_offset_only=True):
        """Exports the register file to a latex table

        :param detail_level: 0: registers only, 1: also show any bit groups
                             2: also show any value tables for individual bit
                             groups
        :type detail_level: int
        :param reg_address: If specified, the returned table is only for a 
                            single register of the register file, as
                            identified by this address (which excludes a base
                            address offset). 
        :type reg_address: int, optional
        :param show_offset_only: If True, it only shows the offset, without 
                                 the base address. 
        :type show_offset_only: bool, optional
        :return: The source code of the latex table
        :rtype: str
        """
        output_text = ''
        temp_base_addr = self._base_address
        temp_base_addr = 0
        hex_digits = int((self._data_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        output_text += r'\endfirsthead'
        output_text += '\n'+r'\endhead'
        output_text += '\n'+r'\hline'
        base_addr_string = ""
        if (not reg_address) and self._base_address and show_offset_only:
            base_addr_string = r" \\(+0x{:02X})".format(self._base_address)
        output_text += '\n'+r'\rowcolor{'+self.__table_colour_head \
            + r'}\shortstack[l]{Address '+base_addr_string+r'} & Name & Description  & POR  \\ \hline'
        is_first = True
        for k in self._regs:
            if reg_address:
                if reg_address != k:
                    continue 
            cur_reg = self._regs[k]
            output_text += ("\n0x{:02X} & {} & {} & "+fmt+r" \\ \hline").format(
                    k*self._reg_offset if not temp_base_addr else (k*self._reg_offset)+self._base_address,
                    self._regs[k]['name'].replace('_', r'\_'),
                    self._regs[k]['description'].replace('_', r'\_'),
                    self._regs[k]['por'])
            if ('bit_groups' in self._regs[k]) and detail_level > 0:
                for b in self._regs[k]['bit_groups']:
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
        return output_text

    def as_html(self,
                 detail_level=0,
                 show_offset_only=True,
                 show_notes=False):
        """Exports the register file to a html table. If internal registers have not already been filtered, then these are styled differently to distinguish them

        :param detail_level: 0: registers only, 1: also show any bit groups
                             2: also show any value tables for individual bit
                             groups
        :type detail_level: int
        :param show_offset_only: If True, it only shows the offset, without 
                                 the base address. 
        :type show_offset_only: bool, optional
        :param show_notes: Whether to show longer, verbose notes specified in register definition
        :type show_notes: bool, optional
        :return: The source code of the html table
        :rtype: str
        """
        output_text = ''
        temp_base_addr = self._base_address
        if show_offset_only:
            temp_base_addr = 0
        output_text += '\n<p>Base: 0x{:02X}</p><p>Data bits: {}</p>'.format(
                self._base_address if self._base_address else 0,
                self._data_bits)
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
        hex_digits = int((self._data_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        for k in self._regs:
            cur_reg = self._regs[k]
            if 'internal' not in self._regs[k]:
                output_text += '\n\n\n<tr class="reg">'
            else:
                if self._regs[k]['internal']:
                    output_text += '\n\n\n<tr class="internal-reg">'
                else:
                    output_text += '\n\n\n<tr class="reg">'
            output_text += ("\n<td>0x{:02X}</td> <td>{} </td><td> {} </td><td> "+fmt+"</td>").format(
                    k*self._reg_offset if not temp_base_addr \
                            else (k*self._reg_offset)+self._base_address,
                    self._regs[k]['name'],
                    self._regs[k]['description'] if not show_notes else \
                            "<p>{}:</p>\n{}".format(
                                    self._regs[k]['description'],
                                    "" if 'notes' not in self._regs[k] \
                                            else "<p>"+("</p><p>".join(self._regs[k]['notes'].split('\n'))).replace('\\t',HTML_TAB)+"</p>" ),
                    self._regs[k]['por'])
            output_text += '\n</tr>\n'
            if ('bit_groups' in self._regs[k]) and detail_level > 0:
                for b in self._regs[k]['bit_groups']:
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

    def get_value_table(self, addr, bit_group):
        """Returns value table from a specific bit group of a specific register. E.g. used to get perf values for M0N0S2 chip

        :param addr: The address of the register containing the relevant bit group (can be the name of the register or address [excluding offset]). 
        :type addr: str, int
        :return: the value table
        :rtype: dict
        """
        return self.read_model(addr,bit_group=bit_group)['value_table']

    def get_base_address(self):
        """Returns the base address of the register file

        :return: the base address
        :rtype: int
        """
        return self._base_address

    def get_reg_string(self, data_base='hex'):
        """Returns a string representation of the register file

        :param data_base: The number system to use to represent the values
                          (allowed values: "bin", "hex", "dec")
        :type data_base: str, optional
        """
        result_str = ""
        if (self._base_address):
            result_str += "base address: 0x{:X}\n".format(self._base_address)
        hex_digits = int((self._data_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        if data_base == 'bin':
            fmt = 'b{:0'+str(self._data_bits)+'b}'
        elif data_base == 'dec':
            fmt = '{:0'+str(len(str(2**self._data_bits)))+'d}'
        return result_str +'\n'.join([("0x{:02X}: "+fmt+"    POR: "+fmt+"    {}").format(
            x, self._regs[x]['val'], self._regs[x]['por'], self._regs[x]['name'])
            for x in sorted([k for k in self._regs])])

    def read_model(self, addr, bit_group=None):
        """Reads a register or bit group value from the model (i.e. not using the read driver)

        :param addr: The register address. Either the name of the register or
                     the numerical address (without base address added). 
        :type addr: int, str
        :param bit_group: Name of the bit group to read. Reads the whole
                          register if not specified.
        :type bit_group: str, optional
        :return: The value as well as the bit group MSB and LSB, the full
                 register value, whether the register is read-only and the
                 value table (where applicable)
        :rtype: dict
        """
        num_addr = None
        if isinstance(addr, (int)):
            if not addr in self._regs:
                raise ValueError(
                    "Error: address (0x{:02X}) not valid!".format(addr))
            res = self._regs[addr]
            num_addr = addr
        elif isinstance(addr, str):
            key = None
            val = None
            for k, v in self._regs.items():
                if v['name'] == addr:
                    key = k
                    val = v
            if key is None:
                raise ValueError("{} not in self._regs".format(addr))
            num_addr = key
            res = val
            # if not addr in [x['name'] for x in self._regs]: # TODO Check
            #  raise ValueError("Reg model does not contain: {}".format(addr))
            #res = [x for x in self._regs if x['name'] == addr]
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
        """Reads a device register directly via a read driver

        :param address: The register absolute address (including the base address)
        :type address: int
        :param driver_name: The name of the read driver to use. If not
                            specified, uses the default read driver
        :type driver_name: str, optional
        """
        self._logger.debug("Read device address: {:0X} using driver: {}".format(
            address,
            driver_name))
        return self._read_driver[driver_name](address)

    def write_device(self, address, data, driver_name='default'):
        """Writes a device register directly via a write driver

        :param address: The register absolute address (including the base address)
        :type address: int
        :param data: Data to write
        :type data: int
        :param driver_name: The name of the write driver to use. If not
                            specified, uses the default write driver
        :type driver_name: str, optional
        """
        self._logger.debug("Write device address: {:0X}, data: {:0X}, using driver: {}".format(
            address,
            data,
            driver_name))
        return self._write_driver[driver_name](address, data)

    def select_bit_range(self, val, msb, lsb):
        """Extracts bit group value from the full register value

        :param val: The register value
        :type val: int
        :param msb: The bit group MSB index
        :type msb: int
        :param lsb: The bit group LSB index
        :type lsb: int
        :return: The value of the bit group
        :rtype: int
        """
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
        """ Writes to a register using the dedicated hardware "set" function (if available, some registers have a hardware-supported SET command if a specific offset is applied to the address). Updates the model. 

        :param addr: The name of the register to set or the numerical
                     relative address (excluding offset)
        :type addr: int, str
        :param data: The data to set
        :type data: int
        """
        self._logger.debug("Write set: A: {}, Data {:X}".format(addr,data))
        if not self._set_addr_offset:
            raise NotImplementedError("This register does not have set function")
        model_rd_data = self.read_model(addr)
        full_reg_val = model_rd_data['full_reg']
        if data > 2**self._data_bits - 1 or data < 0:
            raise ValueError("Error: data larger than data_bits bits!")
        new_val = data
        if isinstance(addr, (int)):
            self._regs[addr]['val'] = model_rd_data['val'] | data
            self.write_device(
                self.get_device_address(addr) + self._set_addr_offset, 
                new_val,
                driver_name=write_driver_name)
        else:  # str
            self._regs[model_rd_data['addr']]['val']  = model_rd_data['val'] | data
            self.write_device(
                self.get_device_address(model_rd_data['addr']) + self._set_addr_offset,
                new_val,
                driver_name=write_driver_name)

    def write_clear(self,
              addr,
              data,
              write_driver_name='default'):
        """ Writes to a register using the dedicated hardware "clear" function (if available, some registers have a hardware-supported CLEAR command if a specific offset is applied to the address). Updates the model. 

        :param addr: The name of the register to clear or the numerical
                     relative address (excluding offset)
        :type addr: int, str
        :param data: The data to clear
        :type data: int
        """
        if not self._clr_addr_offset:
            raise NotImplementedError("This register does not have clear function")
        new_val = data
        model_rd_data = self.read_model(addr)
        full_reg_val = model_rd_data['full_reg']
        if data > 2**self._data_bits - 1 or data < 0:
            raise ValueError("Error: data larger than data_bits bits!")
        if isinstance(addr, (int)):
            self._regs[addr]['val'] = model_rd_data['val'] & (~data)
            self.write_device(
                self.get_device_address(addr) + self._clr_addr_offset, 
                new_val,
                driver_name=write_driver_name)
        else:  # str
            self._regs[model_rd_data['addr']]['val'] = model_rd_data['val'] & (~data)
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
        """Write to a register or bit group via a write driver and update the model. The typical way to write a value to a register. 

        :param addr: The name of the register or numerical relative address
                     (excluding offset) of the register to write to. 
        :type addr: str, int
        :param data: The data value to write
        :type data: int
        :param bit_group: The name of the bit group to write to. If not
                          specified, the whole register is written to
        :type bit_group: str, optional
        :param write_driver: The write driver to use. Uses the default 
                             driver if not specified.
        :type write_driver: str, optional
        :param read_device: Whether to read the value before writing to check
                            the existing modelled value (in case it has been
                            updated externally). (The modelled value is used
                            for masking if writing a bit group)
        :type read_device: bool, optional
        :param read_driver_name: The name of the read driver for checking the
                                 existing modelled value. If not specified, 
                                 the default read driver is used. 
        :type read_driver_name: str, optional
        """
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
                self._regs[addr]['val'] = new_val
                self.write_device(
                    self.get_device_address(addr), 
                    new_val,
                    driver_name=write_driver_name)
            else:  # str
                self._regs[model_rd_data['addr']]['val'] = new_val
                self.write_device(
                    self.get_device_address(model_rd_data['addr']),
                    new_val,
                    driver_name=write_driver_name)
        else:
            # a whole register value
            new_val = data
            if data > 2**self._data_bits - 1 or data < 0:
                raise ValueError("Error: data larger than data_bits bits!")
            if isinstance(addr, (int)):
                if not addr in self._regs:
                    raise ValueError(
                        "Error: address (0x{:02X}) not valid!".format(addr))
                self._regs[addr]['val'] = data
                self.write_device(
                    self.get_device_address(addr), 
                    new_val,
                    driver_name=write_driver_name)
            else:  # str
                self._regs[model_rd_data['addr']]['val'] = data
                self.write_device(
                    self.get_device_address(model_rd_data['addr']),
                    new_val,
                    driver_name=write_driver_name)
        return True

    def read(self,
             addr,
             bit_group=None,
             driver_name='default'):
        """Read from a register or bit group via a read driver. The typical way to read a value from a register. 

        :param addr: The name of the register or numerical relative address
                     (excluding offset) of the register to read from. 
        :type addr: str, int
        :param bit_group: The name of the bit group to read. If not
                          specified, the whole register is read
        :type bit_group: str, optional
        :param driver_name: The name of the read driver to use. If not
                            specified, the default read driver is used
        :return: The read value
        :rtype: int
        """
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
        """Gets all defined addresses in the register file

        :return: The defined (valid) integer relative addresses
        :rtype: list
        """
        return list(self._regs.keys())

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
