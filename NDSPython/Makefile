#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

PATH		:=	$(DSPYTHON)/NDSPython/dldinoap:$(PATH)

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(shell basename $(CURDIR))
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data
INCLUDES	:=	include

#---------------------------------------------------------------------------------
# custom directories
#---------------------------------------------------------------------------------

PARDIR		:=	$(dir $(dir $(CURDIR)))
PDIR		:=  $(dir $(CURDIR))

MOD_SRCDIR	:=	modules/

PYTHON_DIR	:=	$(PARDIR)slp-250
ZLIB_DIR	:=	$(PARDIR)zlib-1.2.3

EXTRA_LIBS	:=	-L$(PYTHON_DIR) -L$(ZLIB_DIR)

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork

# note: arm9tdmi isn't the correct CPU arch, but anything newer and LD
# *insists* it has a FPU or VFP, and it won't take no for an answer!
CFLAGS	:=	-g -Wall -Os\
 			-mcpu=arm9tdmi -mtune=arm9tdmi -fomit-frame-pointer\
			-ffast-math \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9
CXXFLAGS	:= $(CFLAGS)

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	:=	-specs=ds_arm9.specs -g $(ARCH) -mno-fpu -Wl,-Map,$(notdir $*.map) $(EXTRA_LIBS)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lfat -lnds9 -lnds7 -lpython2.5 -lm -lz


#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------

# overwrite ds_rules
LIBNDS := $(DSPYTHON)/libnds-1.3.6
LIBFAT := $(DSPYTHON)/libfat-src-20070127

LIBDIRS	:=	$(LIBNDS) $(LIBFAT)

EXTERNALINCLUDES :=	$(PYTHON_DIR) \
			$(PYTHON_DIR)/include \
			$(PYTHON_DIR)/Stackless

CFLAGS += -I$(DEVKITPRO)/libnds/include
CXXFLAGS += $(CFLAGS)

#LDFLAGS += -L$(DEVKITPRO)/libnds/lib -lfat -lnds9
LDFLAGS += -L$(LIBNDS)/lib -L$(LIBFAT)/nds/lib -lfat -lnds9 -lnds7

EXTRA_OBJS	=	$(MOD_SRCDIR)ndsmodule.o \
			$(MOD_SRCDIR)ndsosmodule.o

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o, $(BINFILES)) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o) \
			$(EXTRA_OBJS)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) $(foreach dir,$(EXTERNALINCLUDES),-I$(dir))

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) $(EXTRA_LIBS)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@[ -d $@/gba_nds_fat ] || mkdir -p $@/gba_nds_fat
	@[ -d $@/modules ] || mkdir -p $@/modules
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).arm9 $(TARGET).ds.gba \
	        $(TARGET)_.ds.gba $(TARGET)_fs.ds.gba $(TARGET)_.nds $(TARGET)_fs.nds 
run:
	wmb -data $(OUTPUT).nds

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(OUTPUT)_fs.ds.gba $(OUTPUT)_fs.nds

$(OUTPUT)_fs.ds.gba	: $(OUTPUT).ds.gba
	cp $(OUTPUT).ds.gba $(OUTPUT)_.ds.gba
	padbin 512 $(OUTPUT)_.ds.gba
	cat $(OUTPUT)_.ds.gba $(PDIR)/python.img > $(OUTPUT)_fs.ds.gba
	dlditool $(PDIR)/fcsr.dldi $(OUTPUT)_fs.ds.gba
	dldinoap $(OUTPUT)_fs.ds.gba
$(OUTPUT)_fs.nds	: $(OUTPUT).nds
	cp $(OUTPUT).nds $(OUTPUT)_.nds
	padbin 512 $(OUTPUT)_.nds
	cat $(OUTPUT)_.nds $(PDIR)/python.img > $(OUTPUT)_fs.nds
	dlditool $(PDIR)/fcsr.dldi $(OUTPUT)_fs.nds
	dldinoap $(OUTPUT)_fs.nds
$(OUTPUT).ds.gba	: 	$(OUTPUT).nds
$(OUTPUT).nds	: 	$(OUTPUT).arm9
$(OUTPUT).arm9	:	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)

#---------------------------------------------------------------------------------
%.jpg.o	:	%.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.raw.o	:	%.raw
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)


-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
