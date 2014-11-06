# $Id: Makefile,v 1.2 2013/12/12 16:12:38 bcbrock Exp $

# This Makefile is designed to be invoked with the -I argument set to
# the location of the "ssx.mk" for the build

# >> gitprep
# Path cleanup for GNU builds
SSX = ..
PGP = $(SSX)/pgp

include $(PGP)/ssx.mk
# << gitprep
include ssxppc405files.mk

ifeq "$(SSX_TIMER_SUPPORT)" "1"
PPC405_OBJECTS += ${PPC405-TIMER-C-SOURCES:.c=.o} ${PPC405-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(SSX_THREAD_SUPPORT)" "1"
PPC405_OBJECTS += ${PPC405-THREAD-C-SOURCES:.c=.o} ${PPC405-THREAD-S-SOURCES:.S=.o}
endif

ifeq "$(PPC405_MMU_SUPPORT)" "1"
PPC405_OBJECTS += ${PPC405-MMU-C-SOURCES:.c=.o} ${PPC405-MMU-S-SOURCES:.S=.o}
endif


all: local
	$(MAKE) -I ../pgp -C ../ppc32

local: $(PPC405_OBJECTS)

.PHONY : clean
clean:
	rm -f *.o *.d *.d.*
	$(MAKE) -I ../pgp -C ../ppc32 clean

ifneq ($(MAKECMDGOALS),clean)
include $(PPC405_OBJECTS:.o=.d) 
endif