# SCUMMVM-PSP MakeFile
# $URL$
# $Id$

#control build
DISABLE_SCALERS = true
DISABLE_HQ_SCALERS = true

srcdir = ../..
VPATH = $(srcdir)
HAVE_GCC3 = false

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
LDFLAGS =

CXX     = psp-g++
AS      = psp-gcc
LD      = psp-gcc
AR      = psp-ar cru
RANLIB  = psp-ranlib
STRIP   = psp-strip
MKDIR 	= mkdir -p
RM 	= rm -f
MKSFO	= mksfo
PACK_PBP = pack-pbp
FIXUP	= psp-fixup-imports

#check PSPSDK presence
ifeq ($(PSPSDK),)
$(error $$(PSPSDK) is undefined.  Use "PSPSDK := $$(shell psp-config --pspsdk-path)" in your Makefile)
endif

# Add in PSPSDK includes and libraries.
INCDIR   := $(srcdir) . $(srcdir)/engines/ . $(PSPSDK)/include
LIBDIR   := $(LIBDIR) . $(PSPSDK)/lib

CFLAGS = -O2 -Wall -D__PSP__ -DNONSTANDARD_PORT -DUSE_ZLIB -Wno-multichar `$(PSPBIN)/sdl-config --cflags` 
CFLAGS	:= $(addprefix -I,$(INCDIR)) $(CFLAGS)
LDFLAGS := $(addprefix -L,$(LIBDIR)) $(LDFLAGS)
LIBS     = 

#comment this out if you don't want to use libmad
CFLAGS	+= -DUSE_MAD
LIBS	+= -lmad 

#comment this out if you don't want to use libtremor
CFLAGS	+= -DUSE_VORBIS -DUSE_TREMOR
LIBS	+= -lvorbisidec

#comment this out if you don't want to use libmpeg2
CFLAGS	+= -DUSE_MPEG2
LIBS	+= -lmpeg2

LIBS    += `$(PSPBIN)/sdl-config --libs` -lz -lstdc++ -lpspdisplay -lpspgu -lpspctrl -lpspsdk -lc -lpspuser -lpspkernel

CXXFLAGS := $(CFLAGS) $(CXXFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  := $(CFLAGS) $(ASFLAGS)

TARGET = scummvm-psp
OBJS := psp_main.o \
	../fs/psp/psp_fs.o \
	osys_psp.o \
	osys_psp_gu.o \
	kbd_ss_c.o \
	kbd_s_c.o \
	kbd_ls_c.o \
	kbd_l_c.o \
	trace.o

MODULE_DIRS += .

include $(srcdir)/Makefile.common

PSP_EBOOT_SFO = param.sfo
PSP_EBOOT_TITLE = ScummVM-PSP
PSP_EBOOT = EBOOT.PBP
PSP_EBOOT_ICON = icon0.png
PSP_EBOOT_ICON1 = NULL
PSP_EBOOT_PIC0 = pic0.png
PSP_EBOOT_PIC1 = pic1.png
PSP_EBOOT_SND0 = NULL
PSP_EBOOT_PSAR = NULL

all: $(PSP_EBOOT)

$(TARGET).elf: $(OBJS)
	$(LD) $^ $(LIBS) -o $@
	$(FIXUP) $@

$(PSP_EBOOT_SFO):
	$(MKSFO) '$(PSP_EBOOT_TITLE)' $@

$(PSP_EBOOT): $(TARGET).elf $(PSP_EBOOT_SFO)
	$(STRIP) $(TARGET).elf
	$(PACK_PBP) $@ $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON) $(PSP_EBOOT_ICON1) \
		$(PSP_EBOOT_PIC0) $(PSP_EBOOT_PIC1) $(PSP_EBOOT_SND0)  \
		$(TARGET).elf $(PSP_EBOOT_PSAR)

kxploit: $(TARGET).elf $(PSP_EBOOT_SFO)
	mkdir -p "$(TARGET)"
	$(STRIP) $(TARGET).elf -o $(TARGET)/$(PSP_EBOOT)
	mkdir -p "$(TARGET)%"
	$(PACK_PBP) "$(TARGET)%/$(PSP_EBOOT)" $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_PIC0) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0) NULL $(PSP_EBOOT_PSAR)

SCEkxploit: $(TARGET).elf $(PSP_EBOOT_SFO)
	mkdir -p "__SCE__$(TARGET)"
	$(STRIP) $(TARGET).elf -o __SCE__$(TARGET)/$(PSP_EBOOT)
	mkdir -p "%__SCE__$(TARGET)"
	$(PACK_PBP) "%__SCE__$(TARGET)/$(PSP_EBOOT)" $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_PIC0) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0) NULL $(PSP_EBOOT_PSAR)

