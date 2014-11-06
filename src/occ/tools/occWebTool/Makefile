# $Id$

#  @file Makefile
#
#  @brief Makefile OCC Error Log Parser application
#

#  @page ChangeLogs Change Logs
#  @section Makefile
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
#  @rc004                  rickylie    03/12/2012   Created
# @ai009   865968          ailutsar    01/09/2013   OCC Error log and trace parser web tool enhancement
#
# @endverbatim
#
CC = g++

INCDIR += -I../../incl
INCDIR += -I../../errl
INCDIR += -I../../trac
INCDIR += -I../../../ssx/

errllog_CFILES = parser.c


all_cfiles = ${errllog_CFILES}
SOURCES = ${all_cfiles}
MODE    = validation

pgas: 
	 $(CC) ${all_cfiles} $(INCDIR) -DOCC_X86_PARSER -Wl,--hash-style=sysv -g -o OCCFileParser
