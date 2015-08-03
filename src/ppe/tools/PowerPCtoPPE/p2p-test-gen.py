#!/usr/bin/python2.6
# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/PowerPCtoPPE/p2p-test-gen.py $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

# \file  p2p-test-gen.py
# \brief this program generates random constructed test cases
#        in the form of input file consumed by ppc-ppe-pcp.py
# \usage create a file named 'test.s' and make sure it has at
#        least one blank line before executing this program.

import fileinput
import random

DotLabel = ['', 'Label:', '.Label']

Comments = ['', '// Comments', '/* Comments */']

TabSpace = ['', '\t', ' ', '\t ', ' \t', ' \t ']

RegLabel = ['', '%r']

Register = [0,1,2,3,4,5,6,7,8,9,10,13,28,29,30,31]

TestEnable = [0,1,2,3]

TestBook = {'eieio'   : 0,
            'isync'   : 0,
            'icbi'    : 0,
            'icbt'    : 0,
            'stbux'   : 3,
            'sthux'   : 3,
            'stwux'   : 3,
            'lbzux'   : 3,
            'lhzux'   : 3,
            'lwzux'   : 3,
            'lha'     : 2,
            'lhau'    : 2,
            'lhax'    : 3,
            'lhaux'   : 3,
            'mulhhw'  : 3,
            'mulhhwu' : 3,
            'mulhw'   : 3,
            'mulhwu'  : 3,
            'mullw'   : 3,
            'mulli'   : 1,
            'divw'    : 3,
            'divwu'   : 3,
            'lmw'     : 2,
            'stmw'    : 2,
            'lwz'     : 4,
            'stw'     : 4,
            'cmplw'   : 5,
            'cmpw'    : 5,
            'cmpwi'   : 5}

BranchList = ['bc', 'bcl', 'blt', 'bltl', 'ble', 'blel', 'bgt', 'bgtl', 'bge',
              'bgel', 'beq', 'beql', 'bne', 'bnel']

def p2p_test():
  for line in fileinput.input('test.s', inplace=1):
    print '// start generating test cases:',
    for opcode,format in TestBook.iteritems():
      opcode += ' '
      if random.randint(1, 10) > 5:
        print random.sample(TabSpace,1)[0] + random.sample(Comments,1)[0] +\
              random.sample(TabSpace,1)[0]
      else:
        print random.sample(TabSpace,1)[0] + random.sample(DotLabel,1)[0] +\
              random.sample(TabSpace,1)[0]
      if format == 0 in TestEnable:
        print random.sample(TabSpace,1)[0] + opcode +\
              random.sample(TabSpace,1)[0] + random.sample(Comments,1)[0]
      if format == 3 in TestEnable:
        regs = random.sample(Register, 3)
        reg_field = random.sample(RegLabel,1)[0] + str(regs[0]) + ',' +\
                    random.sample(TabSpace,1)[0] +\
                    random.sample(RegLabel,1)[0] + str(regs[1]) + ',' +\
                    random.sample(TabSpace,1)[0] +\
                    random.sample(RegLabel,1)[0] + str(regs[2])
        print random.sample(TabSpace,1)[0] + opcode +\
              random.sample(TabSpace,1)[0] + reg_field +\
              random.sample(TabSpace,1)[0] + random.sample(Comments,1)[0]
      if format == 1 in TestEnable:
        regs = random.sample(Register, 2)
        reg_field = random.sample(RegLabel,1)[0] + str(regs[0]) + ',' +\
                    random.sample(TabSpace,1)[0] +\
                    random.sample(RegLabel,1)[0] + str(regs[1]) + ',' +\
                    random.sample(TabSpace,1)[0] +\
                    str(random.randint(-128, 128))
        print random.sample(TabSpace,1)[0] + opcode +\
              random.sample(TabSpace,1)[0] + reg_field +\
              random.sample(TabSpace,1)[0] + random.sample(Comments,1)[0]
      if format == 2 in TestEnable:
        regs = random.sample(Register, 2)
        reg_field = random.sample(RegLabel,1)[0] + str(regs[0]) + ',' +\
                    random.sample(TabSpace,1)[0] +\
                    str(random.randint(-128, 128)) +\
                    '(' + random.sample(RegLabel,1)[0] + str(regs[1]) + ')'
        print random.sample(TabSpace,1)[0] + opcode +\
              random.sample(TabSpace,1)[0] + reg_field +\
              random.sample(TabSpace,1)[0] + random.sample(Comments,1)[0]
      if format == 4 in TestEnable:
        for i in [1,2]:
          regs = random.sample(Register, 2)
          reg_field = random.sample(RegLabel,1)[0] + str(regs[0]) + ',' +\
                      random.sample(TabSpace,1)[0] +\
                      str(random.randint(-128, 128)) +\
                      '(' + random.sample(RegLabel,1)[0] + str(regs[1]) + ')'
          print random.sample(TabSpace,1)[0] + opcode +\
                random.sample(TabSpace,1)[0] + reg_field +\
                random.sample(TabSpace,1)[0] + random.sample(Comments,1)[0]
      if format == 5 in TestEnable:
        if 'i' in opcode:
          regs = random.sample(Register, 1)
          reg_field = random.sample(RegLabel,1)[0] + str(regs[0]) + ',' +\
                      random.sample(TabSpace,1)[0] +\
                      random.sample(RegLabel,1)[0] +\
                      str(random.randint(-128, 128))
        else:
          regs = random.sample(Register, 2)
          reg_field = random.sample(RegLabel,1)[0] + str(regs[0]) + ',' +\
                      random.sample(TabSpace,1)[0] +\
                      random.sample(RegLabel,1)[0] + str(regs[1])
        print random.sample(TabSpace,1)[0] + opcode +\
              random.sample(TabSpace,1)[0] + reg_field +\
              random.sample(TabSpace,1)[0] + random.sample(Comments,1)[0]
        branch = random.sample(BranchList, 1)[0] + ' '
        if 'bc' in branch:
          reg_field = random.sample(TabSpace,1)[0] +\
                      str(random.randint(0, 15)) + ',' +\
                      random.sample(TabSpace,1)[0] +\
                      str(random.randint(0, 7)) + ',' +\
                      random.sample(TabSpace,1)[0] +\
                      str(random.randint(-128, 128)) +\
                      random.sample(TabSpace,1)[0]
        else:
          reg_field = random.sample(TabSpace,1)[0] +\
                      str(random.randint(-128, 128)) +\
                      random.sample(TabSpace,1)[0]
        print random.sample(TabSpace,1)[0] + branch +\
              random.sample(TabSpace,1)[0] + reg_field +\
              random.sample(TabSpace,1)[0] + random.sample(Comments,1)[0]
  fileinput.close()

if __name__ == '__main__':
  p2p_test()



