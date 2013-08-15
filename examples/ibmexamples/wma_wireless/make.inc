ifndef __MAKE_INC__
__MAKE_INC__		:= 1

.PHONY:	all dirs libraries programs install clean
all: dirs libraries programs install

CPP                             := g++
CC 				:= gcc
LD 				:= gcc
RM 				:= rm -f
CP	 			:= cp -pf
MKDIR			:= mkdir -p

COMMON_CFLAGS	:= -W -Wall
_CFLAGS			= $(CFLAGS) $(COMMON_CFLAGS)

get_dir		= $(strip $(subst ./,,$(dir $(1))))
get_name	= $(notdir $(1))

ifneq "$(PROGRAM)" ""
  _PROGRAM_dir	:= $(call get_dir, $(PROGRAM))
  _PROGRAM		:= $(call get_name,$(PROGRAM))
endif

ifneq "$(LIBRARY)" ""
  _LIBRARY_dir	:= $(call get_dir, $(LIBRARY))
  _LIBRARY		:= $(call get_name,$(LIBRARY))
endif

dirs:	   $(DIRS)
programs:  $(_PROGRAM)
libraries: $(_LIBRARY)

%.o: %.c
	$(CC) $(INCLUDE) $(_CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(INCLUDE) $(_CFLAGS) -c $<

ifdef VPATH
  all_src = $(sort $(notdir $(foreach path,. $(subst :, ,$(VPATH)),$(wildcard $(path)/*.$(1)))))
else
  all_src = $(wildcard *.$(1))
endif

C_SRC			:= $(call all_src,c) $(C_BUILT_HERE)
C_OBJS			:= $(C_SRC:.c=.o)

CPP_SRC                   := $(call all_src,cc) $(C_BUILT_HERE)
CPP_OBJS                  := $(CPP_SRC:.cc=.o)

OBJs		:= $(strip $(C_OBJS)) $(strip $(CPP_OBJS))

##### Build Directories #####
ifdef DIRS
.PHONY: $(DIRS)

$(DIRS):
	@if [ -r $@/Makefile ]; then $(MAKE) -C $@ $(MAKECMDGOALS); fi
endif

#####   Build Program   ##### 
ifdef _PROGRAM
$(_PROGRAM): $(OBJs) $(IMPORTS)
	$(LD) -o $@ $(OBJs) $(LDFLAGS) $(IMPORTS)
endif

#####   Build Library   ##### 
ifdef _LIBRARY
$(_LIBRARY): $(OBJs) $(IMPORTS)
	$(AR) -qcs $@ $(OBJs) $(IMPORTS)
endif

#####   Build Install   #####
.SECONDEXPANSION:

ifdef _PROGRAM_dir
$(PROGRAM): $(_PROGRAM)
	@if [ "$(@D)" = "" ]; then echo "Error in PROGRAM definition (use = instead of :=)"; exit 1; fi
	@$(MKDIR) $(@D)
	$(CP) $< $@
endif

ifdef _LIBRARY_dir
$(LIBRARY): $(_LIBRARY)
	@if [ "$(@D)" = "" ]; then echo "Error in LIBRARY definition (use = instead of :=)"; exit 1; fi
	@$(MKDIR) $(@D)
	$(CP) $< $@
endif

ifdef INSTALL_DIR
  _INSTALL_FILES = $(foreach dir,$(INSTALL_DIR),$(addprefix $(dir)/,$(INSTALL_FILES)))
else
  _INSTALL_FILES = $(INSTALL_FILES)
endif

ifdef TARGET_INSTALL_DIR
  _INSTALL_FILES += $(addprefix $(TARGET_INSTALL_DIR)/,$(TARGETS))
endif

ifneq "$(_INSTALL_FILES)" ""
$(_INSTALL_FILES): $$(@F)
	$(MKDIR) $(@D)
	$(CP) $< $@
endif

install: $(_INSTALL_FILES)

#####       Clean       ##### 
cleaninstall:
ifdef _INSTALL_FILES
	@$(RM) $(_INSTALL_FILES)
endif

cleanoutput:
	@$(RM) $(OBJs) $(PROGRAM) $(LIBRARY)

.PHONY: cleanoutput cleansubdirs cleaninstall 
cleansubdirs:
ifdef DIRS
	@for i in $(DIRS); do \
	  if [ -e $$i/Makefile ]; then $(MAKE) -C $$i clean; fi \
	done
endif

clean: cleanoutput cleansubdirs cleaninstall

endif # __MAKE_INC__
