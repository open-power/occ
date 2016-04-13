# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/lib/Makefile $
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
#  This Makefile currently builds a single archive, 'libssx.a', from
#  various library source files. 
#
#  Beware that although this library can be built standalone, parts of
#  its behavior are controlled by options that may be configured in
#  the optional "ssx_app_cfg.h", so it is best to build the library as
#  part of the complete application build.
#
#  The object files stored in the archive are loaded 'on demand'.
#  It may be possible to save a little code space therefore by splitting up
#  the implementation of a header file into multiple files whose
#  functions are not necessarily called together.

SSX = ../ssx
PGP = $(SSX)/pgp
LIB = .

include ./libfiles.mk
include ./libgpefiles.mk
INCLUDES =

include $(PGP)/ssx.mk

libssx.a: $(LIBSSX_OBJECTS)
	$(AR) crs libssx.a $(LIBSSX_OBJECTS)

clean:
	rm -f *.o *.a *.d *.d.* *.lst *.hooks.cc
	rm -f $(TEST_EXECUTABLES)

disassemble: libssx.a
	$(OBJDUMP) -d libssx.a


# The targets 'push_updates_to_lab' and 'diff_with_lab' require that an
# environment variable P8PROCS be defined.  You can also define this on the
# 'make' command line, for example
#
#    make P8PROCS=<... path to your P8 procedures sandbox ...> \
#        push_updates_to_lab

# LIB-TO-PROCS-FILES are files kept here that need to go there, where 'there'
# is the top-level procedures directory.

LIB-TO-PROCS-FILES = pgas.h pgas_ppc.h

push_updates_to_lab:
	if [ -z "$(P8PROCS)" ]; then echo "P8PROCS is not defined"; exit 1; fi
	cp -u $(LIB-TO-PROCS-FILES) $(P8PROCS)

diff_with_lab:
	if [ -z "$(P8PROCS)" ]; then echo "P8PROCS is not defined"; exit 1; fi

	for file in $(LIB-TO-PROCS-FILES); do \
		echo "@@@ $$file"; \
		diff $$file $(P8PROCS)/$$file; \
	done; exit 0


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),push_updates_to_lab)
ifneq ($(MAKECMDGOALS),diff_with_lab)
include $(C-SOURCES:.c=.d) 
include $(S-SOURCES:.S=.d)
include $(if $(filter -DOCC_FIRMWARE=1,$(DEFS)),$(if $(PGAS_PPC),$(pS-SOURCES:.pS=.d),),$(pS-SOURCES:.pS=.d))
endif
endif
endif

