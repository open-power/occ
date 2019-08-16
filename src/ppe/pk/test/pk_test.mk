# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/test/pk_test.mk $
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
IMAGE:=pk_test

$(IMAGE)_TARGET=PPE
_PPE_TYPE=gpe

$(IMAGE)_LINK_SCRIPT=link.ld

PPE_BASELIB=$(ROOTPATH)/chips/p10/common/ppe/baselib
$(call ADD_PPEIMAGE_SRCDIR,$(IMAGE),$(PPE_BASELIB))

include $(PK_SRCDIR)/../ppetrace/pktracefiles.mk
OBJS := $(PKTRACE_OBJECTS)
$(call ADD_PPEIMAGE_SRCDIR,$(IMAGE),$(PK_SRCDIR)/../ppetrace)

OBJS += ppe42_gcc.o
OBJS += ppe42_init.o
OBJS += ppe42_core.o
OBJS += ppe42_irq_core.o
OBJS += ppe42_irq_init.o
OBJS += ppe42_scom.o
OBJS += eabi.o
OBJS += ppe42_math.o
OBJS += ppe42_boot.o
OBJS += ppe42_timebase.o
OBJS += ppe42_thread_init.o
OBJS += ppe42_exceptions.o

include $(PK_SRCDIR)/kernel/pkkernelfiles.mk
OBJS += $(PK_OBJECTS)
OBJS += $(PK_TIMER_OBJECTS)
OBJS += $(PK_THREAD_OBJECTS)
$(call ADD_PPEIMAGE_SRCDIR,$(IMAGE),$(PK_SRCDIR)/kernel)

#include $(PK_SRCDIR)/ppe42/pkppe42files.mk
#OBJS += $(PPE42_OBJECTS)
#OBJS += $(PPE42_THREAD_OBJECTS)
$(call ADD_PPEIMAGE_SRCDIR,$(IMAGE),$(PK_SRCDIR)/ppe42)

include $(PK_SRCDIR)/../boltonlib/$(_PPE_TYPE)/pk$(_PPE_TYPE)files.mk
OBJS += $(GPE_OBJECTS)
$(call ADD_PPEIMAGE_SRCDIR,$(IMAGE),$(PK_SRCDIR)/../boltonlib/$(_PPE_TYPE))

include $(OCC_SRCDIR)/commonlib/libcommonfiles.mk
OBJS += $(LIBCOMMON_OBJECTS)
$(call ADD_PPEIMAGE_SRCDIR,$(IMAGE),$(OCC_SRCDIR)/commonlib)

include $(OCC_SRCDIR)/occlib/liboccfiles.mk
OBJS += $(LIBOCC_OBJECTS)
$(call ADD_PPEIMAGE_SRCDIR,$(IMAGE),$(OCC_SRCDIR)/occlib)

# It's important that the final included *.mk is in the image target src dir
# as it's tracked by the make macros
include $(PK_SRCDIR)/test/pk_testfiles.mk
OBJS += $(PKTEST_OBJECTS)

$(IMAGE)_TRACE_HASH_PREFIX := $(shell echo $(IMAGE) | md5sum | cut -c1-4 \
	| xargs -i printf "%d" 0x{})

$(IMAGE)_COMMONFLAGS+= -DPK_TRACE_LEVEL=3
$(IMAGE)_COMMONFLAGS+= -DPK_THREAD_SUPPORT=1
$(IMAGE)_COMMONFLAGS+= -DPK_TRACE_SUPPORT=1
$(IMAGE)_COMMONFLAGS+= -DUSE_PK_APP_CFG_H=1
$(IMAGE)_COMMONFLAGS+= -D__PPE_PLAT
$(IMAGE)_COMMONFLAGS+= -D__PK__=1
$(IMAGE)_COMMONFLAGS+= -DPBASLVCTLN=0
$(IMAGE)_COMMONFLAGS+= -DAPPCFG_OCC_INSTANCE_ID=1

# add include paths
$(call ADD_PPEIMAGE_INCDIR,$(IMAGE),\
	$(PK_SRCDIR)/test \
	$(PK_SRCDIR)/kernel \
	$(PK_SRCDIR)/ppe42 \
	$(PK_SRCDIR)/../ppetrace \
	$(PK_SRCDIR)/../boltonlib/$(_PPE_TYPE) \
	$(OCC_SRCDIR) \
	$(PMLIB_INCDIR)/registers \
	$(ROOTPATH)/chips/p10/common \
	$(OCC_SRCDIR)/occlib \
	$(ROOTPATH)/chips/p10/common/ppe/powmanlib \
	)

$(IMAGE)_LDFLAGS=-e __system_reset -N -gc-sections -Bstatic


$(call BUILD_PPEIMAGE)
