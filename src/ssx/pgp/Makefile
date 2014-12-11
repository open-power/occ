# $Id: Makefile,v 1.2 2013/12/12 16:12:28 bcbrock Exp $

# This Makefile compiles all of the SSX code required for the PgP port
# of SSX.  See the "ssx.mk" file in this directory.

include ssx.mk
include ssxpgpfiles.mk


ifeq "$(SSX_TIMER_SUPPORT)" "1"
PGP_OBJECTS += ${PGP-TIMER-C-SOURCES:.c=.o} ${PGP-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(SSX_THREAD_SUPPORT)" "1"
PGP_OBJECTS += ${PGP-THREAD-C-SOURCES:.c=.o} ${PGP-THREAD-S-SOURCES:.S=.o}
endif

ifeq "$(PGP_ASYNC_SUPPORT)" "1"
PGP_OBJECTS += ${PGP-ASYNC-C-SOURCES:.c=.o} ${PGP-ASYNC-S-SOURCES:.S=.o}
endif


all: local
	$(MAKE) -I ../pgp -C ../ssx
	$(MAKE) -I ../pgp -C ../ppc405

local: $(PGP_OBJECTS)


.PHONY : clean
clean:
	rm -f *.o *.d *.d.* *.ps *.pdf
	$(MAKE) -I ../pgp -C ../ssx    clean
	$(MAKE) -I ../pgp -C ../ppc405 clean

ifneq ($(MAKECMDGOALS),clean)
include $(PGP_OBJECTS:.o=.d) 
endif
