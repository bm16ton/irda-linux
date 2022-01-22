######################################################################
##                
## Filename:      Makefile
## Version:       
## Description:   Makefile for Linux IrDA Manager
## Status:        Experimental.
## Author:        Dag Brattli <dagb@cs.uit.no>
## Created at:    Thu Feb 19 00:10:23 1998
## Modified at:   Tue Jan 25 09:54:38 2000
## Modified by:   Dag Brattli <dagb@cs.uit.no>
## 
## $Id: Makefile 116 2006-02-28 22:33:43Z sambau $
##
##     Copyright (c) 1998-2000 Dag Brattli, All Rights Reserved.
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
include output.mak

TARGET = test

DIRS = irattach irdaping etc man irnetd psion tekram findchip irdadump smcinit
CFLAGS= -O2 -W -Wall

all:
	@-(set -e ; for d in $(DIRS) ; do $(MAKE) $(MAKE_OUTPUT) -C $$d $@ ; done)


install:
	@-(set -e ; for d in $(DIRS) ; do $(MAKE) $(MAKE_OUTPUT) -C $$d $@ ; done)


clean:
	$(prn_clean)
	@-(set -e ; for d in $(DIRS) ; do $(MAKE) $(MAKE_OUTPUT) -C $$d $@ ; done)


distclean:
	$(prn_distclean)
	@-(set -e ; for d in $(DIRS) ; do $(MAKE) $(MAKE_OUTPUT) -C $$d $@ ; done)

