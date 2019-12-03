# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/ppetrace/ppetracepp/ppetracepp.mk $
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
EXE=ppetracepp
IMAGE_DEPS+=ppetracepp
OBJS=ppetracepp.o
$(call BUILD_EXE)

EXE=ppe-trace
IMAGE_DEPS+=ppe-trace
OBJS=ppeTrace.o ppeTraceEntry.o ppeTrex.o fspTrace.o ppeOpCodes.o
$(call ADD_EXE_INCDIR, $(EXE), \
	$(ROOTPATH)/chips/p10/common/ppe/ppetrace \
	$(ROOTPATH)/chips/p10/procedures/ppe/pgpe \
	)
$(call BUILD_EXE)

