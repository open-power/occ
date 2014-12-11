# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occApplet/testApplet/testappletfiles.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2014
# [+] Google Inc.
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

C-SOURCES = sensorTest.c apsstest.c errlTest.c traceTest.c pstApplet.c
TESTAPLT_OBJECTS = $(C-SOURCES:.c=.o)
sensorTest_OFILES = sensorTest.o
apsstest_OFILES = apsstest.o
errlTest_OFILES = errlTest.o
traceTest_OFILES = traceTest.o
pstApplet_OFILES = pstApplet.o
