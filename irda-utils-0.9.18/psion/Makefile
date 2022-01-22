include ../output.mak

CC=gcc

PSION_SRC= irpsion5.c
PSION_OBJS= irpsion5.o
PSION_TARGETS= irpsion5

all: $(PSION_TARGETS)


clean:
	$(EMCD)rm -f *.o *~ core


distclean:
	$(ECMD)rm -f irpsion5 *.o *~ core


install: $(PSION_TARGETS)
	$(prn_install)
	$(ECMD)install $(PSION_TARGETS) $(ROOT)/usr/bin


CFLAGS += -g -I../include -Wall -Wstrict-prototypes $(RPM_OPT_FLAGS) $(DEB_CFLAGS)
irpsion5: 
	$(prn_cc_o)
	$(ECMD)$(CC) $(DEB_LDFLAGS) $(CFLAGS) $(PSION_SRC) -o $@
