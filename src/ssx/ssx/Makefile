# $Id: Makefile,v 1.2 2013/12/12 16:12:38 bcbrock Exp $

# This Makefile is designed to be invoked with the -I argument set to
# the location of the "ssx.mk" for the build

include ../pgp/ssx.mk
include ssxssxfiles.mk

ifeq "$(SSX_TIMER_SUPPORT)" "1"
SSX_OBJECTS += ${SSX-TIMER-C-SOURCES:.c=.o}
endif

ifeq "$(SSX_THREAD_SUPPORT)" "1"
SSX_OBJECTS += ${SSX-THREAD-C-SOURCES:.c=.o} 
endif

all: $(SSX_OBJECTS)

.PHONY : clean
clean:
	rm -f *.o *.d *.d.*

ifneq ($(MAKECMDGOALS),clean)
include $(SSX_OBJECTS:.o=.d)
endif
