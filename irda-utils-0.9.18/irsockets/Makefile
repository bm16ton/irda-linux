######################################################################
##                
## Filename:      Makefile
## Version:       
## Description:   Makefile for irsockets test programs
## Status:        Experimental.
## Author:        Dag Brattli <dagb@cs.uit.no>
## Created at:    Thu Feb 19 00:10:23 1998
## Modified at:   Sat Dec 25 16:13:54 1999
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

CC = gcc
LD = ld
RM = rm -f

RM_CMD = $(ECMD)$(RM) *.BAK *.bak *.o *.so ,* *~ *.a *.orig *.rej

INCLUDES  = $(SYS_INCLUDES) -I../include
LIBRARIES = $(SYS_LIBRARIES)
LIBPATH   = $(SYS_LIBPATH)

#
# System stuff
#
SYS_INCLUDES = -I/usr/include -I/usr/src/linux/include
SYS_LIBRARIES = 
SYS_LIBPATH = -L/usr/lib -L/usr/local/lib 

CFLAGS= $(SYS_INCLUDES) $(RPM_OPT_FLAGS) -O2 -Wall -Wstrict-prototypes

SRCS = irprintf.c irprintfx.c irscanf.c irscanfx.c irdaspray.c ias_query.c send_ultra.c recv_ultra.c
OBJS = $(SRCS:.c=.o)
TARGETS = irprintf irprintfx irscanf irscanfx irdaspray ias_query send_ultra recv_ultra


all: $(OBJS)


%: %.o
	$(prn_cc_o)
	$(ECMD)$(CC) $(XCFLAGS) -o $@ $^ $(LIBS)


%.o: %.c
	$(prn_cc)
	$(ECMD)$(CC) $(CFLAGS) $(INCLUDES) -c $<


clean:
	$(prn_clean)
	$(RM_CMD) 


distclean:
	$(prn_distclean)
	$(RM_CMD) 
	$(ECMD)$(RM) $(TARGETS)


depend:
	$(prn_depend)
	$(ECMD)makedepend -s "# DO NOT DELETE" -- $(INCLUDES) -- $(SRCS)

# DO NOT DELETE
