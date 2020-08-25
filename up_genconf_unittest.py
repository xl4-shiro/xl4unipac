#!/usr/bin/env python3
'''
/*
 * Excelfore Universal IPC and Configuration code generator
 * Copyright (C) 2020 Excelfore Corporation (https://excelfore.com)
 *
 * This file is part of Excelfore-unipac.
 *
 * Excelfore-unipac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Excelfore-unipac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Excelfore-unipac.  If not, see
 * <https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
 */
'''
import unittest
from up_genconf import *

class TestValueAtom(unittest.TestCase):
    def test_valuse_set(self):
        svs=(\
             ("1234", "int32_t", 0, None, 0),
             ("1234", "uint32_t", 0, True, 0),
             ("1234", "int64_t", 64, False, 0),
             ("1234", "uint64_t", 64, True, 0),
             ("1234L", "int64_t", 0, None, 0),
             ("0x2A", "uint32_t", 0, None, 0),
             ("0x2A", "int32_t", 0, False, 0),
             ("0x2AL", "uint64_t", 0, None, 0),
             ("2A", "uint8_t", 8, True, 0),
             (":2A", "uint8_t", 0, None, 0),
             ("2A:", "uint8_t", 0, None, 0),
             (":2A:", "uint8_t", 0, None, 0),
             ("2A3B","uint16_t", 16, True, 0),
             (":2A3B","uint16_t", 0, None, 0),
             ("2A3B:","uint16_t", 0, None, 0),
             (":2A3B:","uint16_t", 0, None, 0),
             ("'a'", "char", 0, None, 0),
             ('"abcd"', "%s4"%ValueAtom.STRTPREF, 0, None, 0),
             ('"abcd"', "%s3"%ValueAtom.STRTPREF, 0, None, 3),
             ('"abcd"', "%s10"%ValueAtom.STRTPREF, 0, None, 10),
             ("1.23", "double", 0, None, 0),
             ("1.23F", "float", 0, None, 0),
             ("true", "bool", 0, None, 0),
             ("false", "bool", 0, None, 0),
             )
        for sv in svs:
            v=ValueAtom(sv[0], bw=sv[2], unsigned=sv[3], length=sv[4])
            self.assertEqual(v[0], sv[1])

    def test_unit_set(self):
        svs=(\
             ("123,456,789", "{123,456,789}", None, ','),
             ("{123,456,789}", "{{123,456,789}}", None, ','),
             ("{123},{456},{789}", "{{123},{456},{789}}", None, ','),
             ("{123},{456,},{789}", "{{123},{456},{789}}", None, ','),
             ("{123},{456,[2]1},{789}", "{{123},{456,1,1},{789}}", None, ','),
             ("123,{456,789}", "{123,{456,789}}", None, ','),
             ("123,[3]456,789", "{123,456,456,456,789}", None, ','),
             ("12,[2]{34,56},789", "{12,{34,56},{34,56},789}", None, ','),
             ("12:34:56:78:9A", "{12:34:56:78:9A}", 'hex', ':'),
             ("1234:56:789A", "{1234:56:789A}", 'hex', ':'),
             ('"abc","def","x"', '{"abc","def","x"}', None, ','),
             ('"abc","def",[2]"x"', '{"abc","def","x","x"}', None, ','),
             ("'a','b','c'", "{'a','b','c'}", None, ','),
             )

        for sv in svs:
            v=ValueUnit(sv[0], str_format=sv[2], str_delimiter=sv[3])
            self.assertEqual(str(v), sv[1])
            print(v)

        v=ValueUnit('"abc","def","x"')
        for i in v:
            print(i[0])
            self.assertEqual(int(i[0][ValueAtom.STRTPREFLEN:]), len(i[1]))

        v=ValueUnit('1,[10]"abc",2.3,false,{0,1,2},[3]2,{"x","y","z"}')
        print(v)

        v=ValueUnit("1,1L,1.1,1.1F,'a',true,11:22,1111:2222,\"abcd\",\"abcd\"[10],0")
        vl=(4,8,8,4,1,1,1,1,2,2,5,10,4)
        for n,i in enumerate(v):
            self.assertEqual(i.sizeof(), vl[n])

    def test_scanconfig(self):
        dc=ScanConfigFile(fname='sample_defaults.cfg', tfname='sample_defaults.conf')
        dc.proc_allitems()
        self.assertEqual(dc.ditems['VALUE_A_01']['struct'], None)
        self.assertEqual(dc.ditems['VALUE_A_01']['values'], [('int32_t', 1234)])
        self.assertEqual(dc.ditems['VALUE_A_01']['ipcon'], 3)
        self.assertEqual(dc.ditems['VALUE_E_01']['values'], [('uint32_t',36),('uint32_t',86)])
        self.assertEqual(dc.ditems['VALUE_E_02']['values'], [('uint8_t',36),('uint8_t',86)])
        self.assertEqual(dc.ditems['VALUE_E_03']['values'], [('int32_t',24),('int32_t',56)])
        self.assertEqual(dc.ditems['VALUE_F_01']['values'],
                         [('%s2'%ValueAtom.STRTPREF, 'ab'), ('%s3'%ValueAtom.STRTPREF, 'cde'),
                          ('%s2'%ValueAtom.STRTPREF, 'fg')])
        self.assertEqual(dc.ditems['VALUE_F_03']['values'], [('%s9'%ValueAtom.STRTPREF, 'ab'),
                                                             ('%s9'%ValueAtom.STRTPREF, 'e')])
        self.assertEqual(dc.ditems['VALUE_H_02']['values'],
                         [('int32_t', 2),('int32_t', 2),('int32_t', 2),('int32_t', 2)])
        self.assertEqual(dc.ditems['VALUE_H_03']['values'],
                         [('int32_t', 10),('int32_t', 20),('int32_t', 30),('int32_t', 40)])

        self.assertEqual(dc.ditems['VALUE_P']['struct'], 'ABC_01')
        self.assertEqual(dc.ditems['VALUE_P']['values'],
                         [('int32_t', 1), ('%s9'%ValueAtom.STRTPREF, 'abc'), ('double', 2.3),
                          ('bool', False), [('int32_t', 0), ('int32_t', 1), ('int32_t', 2)],
                          ('int32_t', 2), ('int32_t', 2), ('int32_t', 2),
                          [('%s1'%ValueAtom.STRTPREF, 'x'), ('%s2'%ValueAtom.STRTPREF, 'yy'),
                           ('%s1'%ValueAtom.STRTPREF, 'z')]])

        self.assertEqual(dc.ditems['VALUE_Q']['struct'], 'ABC_01')
        self.assertEqual(dc.ditems['VALUE_Q']['values'],
                         [('int32_t', 1), ('%s2'%ValueAtom.STRTPREF, 'xy'), ('double', 3.4)])

        self.assertEqual(dc.ditems['VALUE_R']['struct'], 'ABC_01')
        for i,v in enumerate(dc.ditems['VALUE_R']['values']):
            if i==2 or i==3 or i==4: continue
            self.assertEqual(v,
                         [('int32_t', 10), ('%s1'%ValueAtom.STRTPREF, 'x'), ('double', 1.0),
                          ('bool', True), [('int32_t', 1), ('int32_t', 2), ('int32_t', 3)],
                          ('int32_t', 10), ('int32_t', 20), ('int32_t', 30)])

        v=dc.ditems['VALUE_R']['values'][2]
        self.assertEqual(v,
                         [('int32_t', 0), ('%s1'%ValueAtom.STRTPREF, 'y'), ('double', 2.0),
                          ('bool', True), [('int32_t', 1), ('int32_t', 2), ('int32_t', 3)],
                          ('int32_t', 10), ('int32_t', 20), ('int32_t', 30)])

        v=dc.ditems['VALUE_R']['values'][3]
        self.assertEqual(v,
                         [('int32_t', 10), ('%s1'%ValueAtom.STRTPREF, 'x'), ('double', 1.0),
                          ('bool', False), [('int32_t', 1), ('int32_t', 2), ('int32_t', 3)],
                          ('int32_t', 10), ('int32_t', 20), ('int32_t', 30)])

        v=dc.ditems['VALUE_R']['values'][4]
        self.assertEqual(v,
                         [('int32_t', 2), ('%s4'%ValueAtom.STRTPREF, '1234')])

        for i in range(5):
            v=dc.ditems['VALUE_S']['values'][i]
            self.assertEqual(v,
                             [('int32_t', 3), ('bool', True), ('%s4'%ValueAtom.STRTPREF, 'pq')])

        dc.scan_string_types()
        self.assertEqual(dc.string_types_set, {1,2,3,4,9})

    def test_headeroutput(self):
        pass
        dc=ScanConfigFile(fname='sample_defaults.cfg', tfname='sample_defaults.conf')
        dc.proc_allitems()
        dc.scan_string_types()
        hc=ConfigHeaderOutput(dc, 'sample')
        hc.print_enum()
        hc.print_string_types()
        hc.print_struct()
        hc.write_from_temp("sample_configs.h", "temp_configs.h")
        hc.close()

    def test_codeoutput(self):
        dc=ScanConfigFile(fname='sample_defaults.cfg', tfname='sample_defaults.conf')
        dc.proc_allitems()
        dc.scan_string_types()
        cc=ConfigCodeOutput(dc, 'sample')
        cc.print_config_item_strings()
        cc.print_config_item_ipcon()
        cc.print_config_struct_strings()
        cc.print_config_values()
        cc.print_pointers_list()
        cc.print_struct_field_vtype()
        cc.print_struct_field_update()
        cc.print_get_value()
        cc.write_from_temp("sample_configs.c", "temp_configs.c")
        cc.close()

if __name__=='__main__':
    logger.setLevel(logging.DEBUG)
    unittest.main()
