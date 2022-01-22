######################################################################
##                
## Filename:      Makefile
## Version:       
## Description:   Makefile for Linux IrDA Manager
## Status:        Experimental.
## Author:        Dag Brattli <dagb@cs.uit.no>
## Created at:    Thu Feb 19 00:10:23 1998
## Modified at:   Wed Oct  6 20:22:18 1999
## Modified by:   Dag Brattli <dagb@cs.uit.no>
## 
## $Id: Makefile 115 2006-02-28 22:16:46Z sambau $
##
##     Copyright (c) 1998-1999 Dag Brattli, All Rights Reserved.
##      
##     This program is free software; you can redistribute it and/or 
##     modify it under the terms of the GNU General Public License as 
##     published by the Free Software Foundation; either version 2 of 
##     the License, or (at your option) any later version.
##  
##     Neither Dag Brattli nor University of Tromsø admit liability nor
##     provide warranty for any of this software. This material is 
##     provided "AS-IS" and at no charge.
##     
######################################################################
include ../output.mak

CC           = gcc
LD           = ld
RM           = rm -f
CFLAGS       = -O2 -W -Wall -Wstrict-prototypes -Wshadow $(SYS_INCLUDES)

SYS_INCLUDES = -I/usr/include
SYS_LIBPATH  = -L/usr/lib -L/usr/local/lib 

INCLUDES     = $(SYS_INCLUDES) -I../include/
LIBRARIES    = $(SYS_LIBRARIES)
LIBPATH      = $(SYS_LIBPATH)

TARGETS      = irattach dongle_attach

all: $(TARGETS)


.c.o:
	$(prn_cc)
	$(ECMD)$(CC) $(CFLAGS) $(INCLUDES) -c $<


irattach: irattach.o util.o
	$(prn_cc_o)
	$(ECMD)$(CC) $(CFLAGS) irattach.o util.o -o $@



dongle_attach: dongle_attach.o
	$(prn_cc_o)
	$(ECMD)$(CC) $(CFLAGS) dongle_attach.o -o $@


install: $(TARGETS)
	$(prn_install)
	$(ECMD)install irattach $(ROOT)/usr/sbin
	$(ECMD)install dongle_attach $(ROOT)/usr/sbin


clean: 
	$(ECMD)$(RM) *.BAK *.bak *.o ,* *~ *.a


distclean:
	$(ECMD)$(RM) *.BAK *.bak *.o ,* *~ *.a
	$(ECMD)$(RM) irattach dongle_attach


tags:
	$(prn_etags)
	$(ECMD)etags *.[hc] */*.[hc] */*/*.[hc]


depend:
	$(prn_depend)
	$(ECMD)makedepend -s "# DO NOT DELETE" -- $(INCLUDES) -- $(SRCS)

# DO NOT DELETE
