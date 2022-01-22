ifeq ("$(V)", "1")
EPRN = > /dev/null
ECMD =
MAKE_OUTPUT = 
else
EPRN =
ECMD = @
MAKE_OUTPUT = -s
endif

prn_cc        = @echo " [CC] " $< $(EPRN)
prn_cc_o      = @echo " [CC] " $@ $(EPRN)
prn_ar        = @echo " [AR] " $@ $(EPRN)
prn_ranlib    = @echo " [RANLIB] " $@ $(EPRN)
prn_clean     = @echo " [CLEAN] " $(EPRN)
prn_distclean = @echo " [DISTCLEAN] " $(EPRN)
prn_etags     = @echo " [ETAGS] " $@ $(EPRN)
prn_install   = @echo " [INSTALL] " $< $(EPRN)
prn_depend    = @echo " [DEPEND]" $(EPRN)
prn_man       = @echo " [DOCBOOK]" $< $(EPRN)
prn_gzip      = @echo " [GZIP]" $< $(EPRN)
