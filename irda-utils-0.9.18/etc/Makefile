include ../output.mak

CC = gcc $(RPM_OPT_FLAGS)
LD = ld
RM = rm -f

RM_CMD = $(RM) *.BAK *.bak *.o ,* *~ *.a

all:

etc: all

clean:
	$(RM_CMD)

distclean:
	$(RM_CMD)

install: etc
	$(prn_install)
	@./install-etc $(ROOT)
