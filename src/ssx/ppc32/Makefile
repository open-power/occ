# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/ppc32/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2014,2016
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
# This Makefile is designed to be invoked with the -I argument set to
# the location of the "ssx.mk" for the build.
#
# This makefile creates the libppc32.a library.

SSX = ..
PGP = $(SSX)/pgp

include $(PGP)/ssx.mk
include ssxppc32files.mk



libppc32.a: ${PPC32_OBJECTS}
	$(AR) crs libppc32.a ${PPC32_OBJECTS}

.PHONY : clean
clean:
	rm -f *.o *.a *.d *.d.*


#  This clause prevents the dependencies from creating errors during a clean.
#  Whenever a header file is added or deleted it will likely be necessary to
#  'make clean' to force recomputation of dependencies.

ifneq ($(MAKECMDGOALS),clean)
include $(PPC32_OBJECTS:.o=.d) 
endif
