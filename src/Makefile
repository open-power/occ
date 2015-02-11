
ifndef ROOTPATH
ROOTPATH = $(shell pwd)
export OCCROOT = $(ROOTPATH)
endif

#*******************************************************************************
# mk variable Declaration
#*******************************************************************************
SUBDIRS = tracepp occBootLoader occ occApplet
COMBINE_IMAGE_SUBDIRS = occBootLoader occ occApplet
CLEAN_SUBDIRS = occBootLoader occ occApplet ssx/pgp lib
CLEANCMD = $(MAKE) clean -C $(dir); 
ALLCMD = $(MAKE) -C $(dir);
COMBINEIMAGE = $(MAKE) combineImage -C $(dir);
IMAGEFILE = image.bin

export TRACEPP = $(ROOTPATH)/tracepp/tracepp
TRACEHASH = $(ROOTPATH)/tracepp/tracehash.pl

#*******************************************************************************
# Compilation 
#*******************************************************************************
all:
	$(foreach dir,$(SUBDIRS),$(ALLCMD))

#*******************************************************************************
# Combine Objects/Executables into OCC Image
#*******************************************************************************
.PHONY : combineImage
combineImage:
	 rm -rf $(IMAGEFILE)
	 $(foreach dir,$(COMBINE_IMAGE_SUBDIRS),$(COMBINEIMAGE))

#*******************************************************************************
# Create lid or ruhx out of OCC Image
#*******************************************************************************
.PHONY : 81e004%.ruhx
81e004%.ruhx:
	occgenlid.pl -i ${IMAGEFILE} -l $(basename $@)

.PHONY : 81e004%.lid
81e004%.lid:
	cp ${IMAGEFILE} $(dir $(IMAGEFILE))/$@

#*******************************************************************************
# Create occStringFile
#*******************************************************************************
# FIXME depending on where .o files end up
.PHONY : tracehash
tracehash:
	$(TRACEHASH) -c -d . -s $(ROOTPATH)/occStringFile

#*******************************************************************************
# Create occcSrcParse.C
#*******************************************************************************
# FIXME error log parser plugins not relevant to OpenPOWER, tool location unknown
.PHONY : srcPluginFile
srcPluginFile:
	scanforsrcs.pl

#*******************************************************************************
# Clean
#*******************************************************************************
clean: 
	$(foreach dir,$(CLEAN_SUBDIRS),$(CLEANCMD))
	rm -rf image.bin
	
