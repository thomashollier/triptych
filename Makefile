# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.
ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
    OF_ROOT=../../..
endif



# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk



APPDIR = $(shell pwd)
DSTDIR = "/home/pi/bin/"
y  = $(subst _, , $(APPNAME))
APPBASENAME = $(word 1, $(y))


install: 
	@echo $(APPNAME) 
	@cp $(APPDIR)/bin/$(APPNAME) $(DSTDIR)
	@ln -sf $(DSTDIR)/$(APPNAME) $(DSTDIR)/$(APPBASENAME) 
	@rsync -rav $(DSTDIR) pi2:$(DSTDIR)
	@rsync -rav $(DSTDIR) pi3:$(DSTDIR)
	@rsync -rav "/home/pi/playback_settings.xml" pi2:"/home/pi/playback_settings.xml"  
	@rsync -rav "/home/pi/playback_settings.xml" pi3:"/home/pi/playback_settings.xml"  
	@ssh pi2 'ln -sf $(DSTDIR)/$(APPNAME) $(DSTDIR)/$(APPBASENAME)'
	@ssh pi3 'ln -sf $(DSTDIR)/$(APPNAME) $(DSTDIR)/$(APPBASENAME)'

.PHONY: install 
	#@ssh pi2 'killall $(APPNAME)'
	#@ssh pi3 'killall $(APPNAME)'

