# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/test/pk_testfiles.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2019
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
ifdef IMAGE

PKTEST-C-SOURCES = pk_app_irq_table.c
PKTEST-C-SOURCES += ipc_func_tables.c
PKTEST-C-SOURCES += main.c
PKTEST-CXX-SOURCES =
PKTEST-S-SOURCES =

PKTEST_OBJECTS = $(PKTEST-C-SOURCES:.c=.o)
PKTEST_OBJECTS += $(PKTEST-S-SOURCES:.S=.o)
PKTEST_OBJECTS += $(PKTEST-CXX-SOURCES:.C=.o)

endif
