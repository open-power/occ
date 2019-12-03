# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/ppetrace/ppetracepp/Makefile $
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
all: ppetracepp ppe2fsp ppe-trace

ppetracepp: ppetracepp.C
	g++ -O3 -w -g -I./ ppetracepp.C -o ppetracepp
#	g++ -O3 -w -x c++ -fPIC -g -I./ ppetracepp.C -o ppetracepp

ppe2fsp: ppe2fsp.c ppe2fsp_cmd.c
	gcc -w -g -I./ -I../ ppe2fsp.c ppe2fsp_cmd.c -o ppe2fsp

ppe-trace: ppeTrace.C ppeTraceEntry.C ppeTrex.C ppeOpCodes.C
	g++ -g -I./ -I../ ppeTrace.C ppeTraceEntry.C ppeTrex.C fspTrace.C \
		ppeOpCodes.C \
		-o ppe-trace

clean:
	rm ppetracepp ppe2fsp ppe-trace

