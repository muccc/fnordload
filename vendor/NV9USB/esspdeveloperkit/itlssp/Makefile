# -*- mode: Makefile; -*-
# -----------------------------------------
# project itlssptest


all: all.targets
	
all.targets : 
	$(MAKE) -C lib
	$(MAKE) -C BasicValidator 
	$(MAKE) -C DownloadValidator
	$(MAKE) -C ValidatorInfo
 
clean :
	cd lib && $(MAKE) clean
	cd BasicValidator && $(MAKE) clean
	cd DownloadValidator && $(MAKE) clean
	cd ValidatorInfo && $(MAKE) clean

.PHONY: all clean distclean

