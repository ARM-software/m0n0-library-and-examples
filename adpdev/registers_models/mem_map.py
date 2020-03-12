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

class MemoryMap:
    def __init__(self, map_config, logger=None):
        self._logger = logger or logging.getLogger(__name__)
        self._logger.info('Creating memory map with "{}"'.format(map_config))
        self._map = None
        self._map_name = map_config
        if map_config.lower().endswith('.yml') or \
                map_config.lower().endswith('.yaml'):
            import yaml
            with open(map_config, 'r') as f:
                temp = yaml.safe_load(f)
                self._table_colour_head = 'ArmBlue'
                self._table_colour_l1 = 'white'
                self._table_colour_l2 = 'ArmLightGray'
                self._table_colour_l3 = 'ArmDarkGray'
                self._map = temp['submaps']
                self._map_base = temp['base']
                self._map_size = temp['size']
                self._map_top = temp['base'] + temp['size']
                self._address_bits = len(bin(temp['base'] + temp['size'])[2:])
                self._logger.debug("Mem map address bits: " +
                                   str(self._address_bits))
            f.closed
        else:
            raise IOError(
                "map_config ({}) has wrong extension".format(map_config))

    def get_attr(self, region, attr):
        if isinstance(region, str):
            res = [x[attr] for x in self._map if x['name'] == region]
            if len(res) == 1:
                return res[0]
            else:
                raise ValueError("Length of res != 1, {}".format(res))
        else:  # list
            cur_el = self._map
            for k in range(0, len(region)):
                res = [x for x in cur_el if x['name'] == region[k]]
                if k < len(region)-1:
                    cur_el = res[0]['submaps']
                else:
                    cur_el = res[0]
            return cur_el[attr]

    def get_base(self, region):
        return self.get_attr(region,'base')

    def get_size(self, region):
        return self.get_attr(region,'size')

    def get_map_string(self, data_base='hex'):
        hex_digits = int((self._address_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        if data_base == 'bin':
            fmt = 'b{:0'+str(self._address_bits_bits)+'b}'
        elif data_base == 'dec':
            fmt = '{:0'+str(len(str(2**self._address_bits)))+'d}'
        return '\n'.join([("{:<16}: "+fmt+": size: "+fmt+"").format(
            self._map[x]['name'],
            self._map[x]['base'],
            self._map[x]['size'],
        )
            for x in range(0, len(self._map))])

    def as_code_constants(self,map_name):
        lf = line_format.Line_Format(map_name,60,8)
        output_text = ''
        output_text += '\n\n// '+map_name
        output_text += '\n// Generated from: '+self._map_name+'\n'
        for k in range(0, len(self._map)):
            b = self._map[k]
            output_text += lf.fmt(
                    b['name'],
                    b['base'],
                    postfix='BASE')
            output_text += lf.fmt(
                    b['name'].upper(),
                    b['size'],
                    postfix='SIZE')
            if ('submaps' in b):
                for s in range(0, len(b['submaps'])):
                    sub = b['submaps'][s]
                    output_text += lf.fmt(
                        b['name']+"_"+sub['name'],
                        sub['base'],
                        indent=2,
                        postfix='BASE')
                    output_text += lf.fmt(
                        b['name']+"_"+sub['name'],
                        sub['size'],
                        indent=2,
                        postfix='SIZE')
        return output_text

    def as_latex(self,
                 detail_level=0,
                 region_name=None):
        output_text = ''
        #output_text += '\n'+r'\begin{longtable}{|l|p{6.5cm}|l|l|}'
        output_text += r'\endfirsthead'
        output_text += '\n'+r'\endhead'
        output_text += '\n'+r'\hline'
        output_text += '\n'+r'\rowcolor{'+self._table_colour_head \
            + r'}Name & Description & Address  & Size  \\ \hline'
        hex_digits = int((self._address_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        for k in range(0, len(self._map)):
            if region_name:
                if region_name != self._map[k]['name']:
                    continue
            cur_reg = self._map[k]
            output_text += ("\n{} & {} & "+fmt+" & "+fmt+r" \\ \hline").format(
                cur_reg['name'].replace('_', r'\_'),
                cur_reg['description'].replace('_', r'\_'),
                cur_reg['base'],
                cur_reg['size'])
            if ('submaps' in cur_reg) and detail_level > 0:
                for s in range(0, len(cur_reg['submaps'])):
                    sub = cur_reg['submaps'][s]
                    output_text += '\n'+r'\rowcolor{'+self._table_colour_l2+'}'
                    temp_description = sub['description'].replace('_', r'\_') if \
                        'description' in sub else ' '
                    output_text += r'\relax' + '\n &  ' \
                        + sub['name'].replace('_', r'\_')+'  & ' \
                        + (""+fmt+" & " +
                           fmt).format(sub['base'], sub['size'])
                    output_text += r' \\ \hline'
        return output_text

    def as_html(self,
                 detail_level=0,
                 region_name=None):
        output_text = ''
        output_text += '\n<table>\n'
        output_text += '<tr>\n'
        output_text += '    <th>Name</th>\n'
        output_text += '    <th>Description</th>\n'
        output_text += '    <th>Address</th>\n'
        output_text += '    <th>Size</th>\n'
        output_text += '</tr>'
        hex_digits = int((self._address_bits / 4)+0.5)
        fmt = '0x{:0'+str(hex_digits)+'X}'
        for k in range(0, len(self._map)):
            if region_name:
                if region_name != self._map[k]['name']:
                    continue
            output_text += '\n\n\n<tr class="reg">'
            cur_reg = self._map[k]
            output_text += ("\n<td>{}</td><td>{}</td><td>"+fmt+"</td><td>"
                    +fmt+"</td>").format(
                cur_reg['name'],
                cur_reg['description'],
                cur_reg['base'],
                cur_reg['size'])
            output_text += '\n</tr>\n'
            if ('submaps' in cur_reg) and detail_level > 0:
                for s in range(0, len(cur_reg['submaps'])):
                    sub = cur_reg['submaps'][s]
                    output_text += '\n<tr>'
                    temp_description = sub['description'] if \
                        'description' in sub else ' '
                    output_text += '\n <td></td>  ' \
                        + '<td>'+sub['name']+'</td><td> ' \
                        + (""+fmt+" </td> <td> " +
                           fmt+'</td>').format(sub['base'], sub['size'])
                    output_text += '</tr>'
        output_text += '</table>'
        return output_text



    def __str__(self):
        return self.get_map_string()


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(
        description='Mem Map Model')
    parser.add_argument('-m', '--map-config', required=True,
                        help="Mem Map configuration file")
    args = parser.parse_args()
    mem_map = MemoryMap(args.map_config)
    print(mem_map)
    print(mem_map.as_latex(detail_level=1))
    '''
  print("Now printing markdown")
  with open('temp.md','w') as f:
    f.write(my_reg.as_markdown(detail_level=2))
  f.closed
  with open('temp.tex','w') as f:
    f.write(my_reg.as_latex(detail_level=2))
  f.closed
  my_reg.add_unused_bits_and_order()
  with open('temp-after-reorder.tex','w') as f:
    f.write(my_reg.as_latex(detail_level=2))
  f.closed
  print(my_reg.register_detail_as_latex(0x8))
  '''
