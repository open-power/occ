# $Id: Makefile,v 1.2 2013/12/12 16:12:37 bcbrock Exp $

# This Makefile is designed to be invoked with the -I argument set to
# the location of the "ssx.mk" for the build.
#
# This makefile creates the libppc32.a library.

# >> gitprep
# Path cleanup for GNU builds
SSX = ..
PGP = $(SSX)/pgp

include $(PGP)/ssx.mk
# << gitprep
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