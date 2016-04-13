# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/pgp/ssxpgpfiles.mk $
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
#  @file ssxpgpfiles.mk
#
#  @brief mk for including pgp object files
#
#  @page ChangeLogs Change Logs
#  @section ssxpgpfiles.mk
##########################################################################
# Object Files
##########################################################################
PGP-C-SOURCES = pgp_init.c pgp_irq_init.c pgp_pmc.c pgp_ocb.c pgp_pba.c \
	pgp_id.c pgp_centaur.c
PGP-S-SOURCES = pgp_cache.S

PGP-TIMER-C-SOURCES = 
PGP-TIMER-S-SOURCES = 

PGP-THREAD-C-SOURCES = 
PGP-THREAD-S-SOURCES = 

PGP-ASYNC-C-SOURCES = pgp_async.c pgp_async_pore.c pgp_async_ocb.c \
	pgp_async_pba.c
PGP-ASYNC-S-SOURCES =

PGP_OBJECTS += $(PGP-C-SOURCES:.c=.o) $(PGP-S-SOURCES:.S=.o)

