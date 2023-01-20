# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_gpe1/topfiles.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015,2023
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

TOP-C-SOURCES = gpe1_main.c nop.c \
                pk_app_irq_table.c ipc_func_tables.c \
                gpe1_24x7.c gpe_gpu_init.c \
                gpe_membuf_scom.c \
                gpe_membuf.c occ_gpe1_machine_check_handler.c \
                gpe1_dimm_read.c gpe1_dimm_reset.c \
                gpe_ocmb.c

TOP-S-SOURCES = occ_gpe1_mck_handler.S

TOP_OBJECTS = $(TOP-C-SOURCES:.c=.o) $(TOP-S-SOURCES:.S=.o)
