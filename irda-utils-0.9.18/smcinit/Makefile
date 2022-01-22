include ../output.mak

CC = gcc
RM = rm -f

RM_CMD = $(ECMD)$(RM) *.BAK *.bak *.o ,* *~ *.a

INCLUDES  = $(SYS_INCLUDES) -I../include/
LIBRARIES = $(SYS_LIBRARIES)
LIBPATH   = $(SYS_LIBPATH)

#
# System stuff
#
SYS_INCLUDES = -I/usr/include
SYS_LIBRARIES = 
SYS_LIBPATH = -L/usr/lib -L/usr/local/lib 

CFLAGS= $(DEB_CFLAGS) -W -Wall -I.

SMCINIT_OBJS = smcinit.o tosh1800-smcinit.o tosh2450-smcinit.o 

SMCINIT_TARGETS = smcinit tosh1800-smcinit tosh2450-smcinit

all: $(SMCINIT_TARGETS)

smcinit: smcinit.o
	$(prn_cc_o)
	$(ECMD)$(CC) $(DEB_LDFLAGS) -o $@ $^ $(LIBS) -lpci


tosh1800-smcinit: tosh1800-smcinit.o
	$(prn_cc_o)
	$(ECMD)$(CC) $(DEB_LDFLAGS) $(XCFLAGS) -o $@ $^ $(LIBS) -lpci


tosh2450-smcinit: tosh2450-smcinit.o
	$(prn_cc_o)
	$(ECMD)$(CC) $(DEB_LDFLAGS) $(XCFLAGS) -o $@ $^ $(LIBS) -lpci


%o: %c
	$(prn_cc)
	$(ECMD)$(CC) $(CFLAGS) $(INCLUDES) -c $<

install: $(SMCINIT_TARGETS)
	$(prn_install)
	$(ECMD)install smcinit $(ROOT)/usr/sbin
	$(ECMD)install tosh1800-smcinit $(ROOT)/usr/sbin
	$(ECMD)install tosh2450-smcinit $(ROOT)/usr/sbin

clean:
	$(RM_CMD)

distclean:
	$(RM_CMD)
	$(ECMD)$(RM) $(SMCINIT_TARGETS)
