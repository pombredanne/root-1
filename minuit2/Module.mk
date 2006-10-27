# Module.mk for minuit2 module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Rene Brun, 07/05/2003

MODDIR        := minuit2
MODDIRS       := $(MODDIR)/src
MODDIRI       := $(MODDIR)/inc

MINUIT2DIR    := $(MODDIR)
MINUIT2DIRS   := $(MINUIT2DIR)/src
MINUIT2DIRI   := $(MINUIT2DIR)/inc

MINUITBASEVERS := Minuit-1_7_6
MINUITBASESRCS := $(MODDIRS)/$(MINUITBASEVERS).tar.gz
MINUITBASEDIRS := $(MODDIRS)/$(MINUITBASEVERS)
MINUITBASEDIRI := -I$(MODDIRS)/$(MINUITBASEVERS)
MINUITBASEETAG := $(MODDIRS)/headers.d

##### liblcg_Minuit #####
ifeq ($(PLATFORM),win32)
MINUITBASELIBA      := $(MINUITBASEDIRS)/libminuitbase.lib
MINUITBASELIB       := $(LPATH)/libminuitbase.lib
ifeq (debug,$(findstring debug,$(ROOTBUILD)))
MINUITBASEBLD        = "DEBUG=1"
else
MINUITBASEBLD        = ""
endif
else
MINUITBASELIBA      := $(MINUITBASEDIRS)/src/.libs/liblcg_Minuit.a
MINUITBASELIB       := $(LPATH)/libminuitbase.a
endif
MINUITBASEDEP       := $(MINUITBASELIB)

##### libMinuit2 #####
MINUIT2L     := $(MODDIRI)/LinkDef.h
MINUIT2DS    := $(MODDIRS)/G__Minuit2.cxx
MINUIT2DO    := $(MINUIT2DS:.cxx=.o)
MINUIT2DH    := $(MINUIT2DS:.cxx=.h)

MINUIT2AH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
MINUIT2BH    := $(filter-out $(MODDIRI)/Minuit2/LinkDef%,$(wildcard $(MODDIRI)/Minuit2/*.h))
MINUIT2H     := $(MINUIT2AH) $(MINUIT2BH)
MINUIT2S     := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MINUIT2O     := $(MINUIT2S:.cxx=.o)

MINUIT2DEP   := $(MINUIT2O:.o=.d) $(MINUIT2DO:.o=.d)

MINUIT2LIB   := $(LPATH)/libMinuit2.$(SOEXT)

# use this compiler option if want to optimize object allocation in Minuit2
# NOTE: using this option one loses the thread safety. 
# It is worth to use only for minimization of cheap (non CPU intensive) functions
#CXXFLAGS += -DMN_USE_STACK_ALLOC

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(MINUIT2H))
ALLLIBS      += $(MINUIT2LIB)

# include all dependency files
INCLUDEFILES += $(MINUIT2DEP)

##### local rules #####
include/Minuit2/%.h: $(MINUIT2DIRI)/Minuit2/%.h
		@(if [ ! -d "include/Minuit2" ]; then     \
		   mkdir -p include/Minuit2;              \
		fi)
		cp $< $@

include/%.h:    $(MINUIT2DIRI)/%.h
		cp $< $@


$(MINUIT2LIB):  $(MINUIT2O) $(MINUIT2DO) $(ORDER_) $(MAINLIBS) $(MINUIT2LIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libMinuit2.$(SOEXT) $@ \
		   "$(MINUIT2O) $(MINUIT2DO)" \
		   "$(MINUIT2LIBEXTRA)"

$(MINUIT2DS):   $(MINUIT2H) $(MINUIT2L) $(ROOTCINTTMPEXE) 
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(MINUIT2H) $(MINUIT2L)

all-minuit2:    $(MINUIT2LIB)

map-minuit2:    $(RLIBMAP)
		$(RLIBMAP) -r $(ROOTMAP) -l $(MINUIT2LIB) \
		   -d $(MINUIT2LIBDEP) -c $(MINUIT2L)

map::           map-minuit2

test-minuit2: 	$(MINUIT2LIB)
		cd $(MINUIT2DIR)/test; make

clean-minuit2:
		@rm -f $(MINUIT2O) $(MINUIT2DO)

clean::         clean-minuit2

distclean-minuit2: clean-minuit2
		@rm -f $(MINUIT2DEP) $(MINUIT2DS) $(MINUIT2DH) $(MINUIT2LIB)
		@rm -rf include/Minuit2

distclean::     distclean-minuit2
