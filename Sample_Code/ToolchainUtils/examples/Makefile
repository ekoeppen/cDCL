NCT_PROJECTS    ?= ../../../../NewtonDev/NCT_Projects

CC              := arm-none-eabi-gcc
CXX             := arm-none-eabi-g++
AS              := arm-none-eabi-as
LINKER          := arm-none-eabi-ld
OBJCOPY         := arm-none-eabi-objcopy

MACHINEFLAGS    := -march=armv4 -mbig-endian
ASFLAGS         := ${MACHINEFLAGS}
CODEGENFLAGS    := -nostdinc -fPIC -fdata-sections -ffunction-sections -O3 -Os
DEFINES         := -D__arm -DhasPCMCIA -DforARM -DNO_ERRNO_H -DSTDC
CFLAGS          := ${MACHINEFLAGS} ${CODEGENFLAGS} ${DEFINES}
CXXFLAGS        := ${MACHINEFLAGS} ${CODEGENFLAGS} ${DEFINES} -fno-exceptions
OBJDIR          := obj

NEWTONINCLUDES  := -I$(NCT_PROJECTS)/DDKIncludes -I$(NCT_PROJECTS)/DDKIncludes/Bootstrap -I$(NCT_PROJECTS)/DDKIncludes/CLibrary -I$(NCT_PROJECTS)/DDKIncludes/CommAPI -I$(NCT_PROJECTS)/DDKIncludes/Communications -I$(NCT_PROJECTS)/DDKIncludes/Frames -I$(NCT_PROJECTS)/DDKIncludes/HAL -I$(NCT_PROJECTS)/DDKIncludes/Lantern -I$(NCT_PROJECTS)/DDKIncludes/OS600 -I$(NCT_PROJECTS)/DDKIncludes/Packages -I$(NCT_PROJECTS)/DDKIncludes/Power -I$(NCT_PROJECTS)/DDKIncludes/QD -I$(NCT_PROJECTS)/DDKIncludes/Toolbox -I$(NCT_PROJECTS)/DDKIncludes/UtilityClasses -I$(NCT_PROJECTS)/DDKIncludes/PCMCIA -I$(NCT_PROJECTS)/Includes -I$(NCT_PROJECTS)/Includes/CLibrary -I$(NCT_PROJECTS)/Includes/Frames -I$(NCT_PROJECTS)/Includes/Utilities -I$(NCT_PROJECTS)/LanternDriver_DDK/Includes -I$(NCT_PROJECTS)/NSandDDK

RUNTIMESRCDIR   := NewtonRuntime/
RUNTIMECXXSRCS  := $(shell find $(RUNTIMESRCDIR) -type f -name "*.cpp")
RUNTIMEASMSRCS  := $(shell find $(RUNTIMESRCDIR) -type f -name "*.s")
RUNTIMECXXOBJS  := $(patsubst $(RUNTIMESRCDIR)/%,$(OBJDIR)/%,$(RUNTIMECXXSRCS:.cpp=.cpp.o))
RUNTIMEASMOBJS  := $(patsubst $(RUNTIMESRCDIR)/%,$(OBJDIR)/%,$(RUNTIMEASMSRCS:.s=.s.o))

VPATH           := $(RUNTIMESRCDIR)

all : $(OBJDIR) ntkmodule.ntkc protocol.pkg

clean :
	rm -rf $(OBJDIR)
	rm -f ntkmodule.ntkc
	rm -f protocol.pkg

$(OBJDIR) :
	mkdir $(OBJDIR)

$(RUNTIMECXXOBJS) : $(OBJDIR)/%.cpp.o : %.cpp
	$(CXX) $(CXXFLAGS) $(NEWTONINCLUDES) -c $< -o $@

$(RUNTIMEASMOBJS) : $(OBJDIR)/%.s.o : %.s
	$(AS) $(ASFLAGS) -c $< -o $@

# NTKC rules

NTKCCXXOBJS := $(OBJDIR)/ntkmodule.cpp.o
NTKCASMOBJS := $(OBJDIR)/PDemoInterfaceGlue.s.o
NTKCLINKFLAGS   := -EB -shared --gc-sections --no-allow-shlib-undefined --no-export-dynamic -x --no-undefined -Tntkmodule.ld --version-script=ntkmodule.map

ntkmodule.ntkc : $(OBJDIR)/ntkmodule.elf ntkmodule.exp
	rm -f $@
	ELFtoNTK -via ntkmodule.exp -o $@ $(OBJDIR)/ntkmodule.elf
	SetFile -c NTP1 -t ntkc $@

$(OBJDIR)/ntkmodule.elf : $(NTKCCXXOBJS) $(NTKCASMOBJS) $(RUNTIMECXXOBJS) $(RUNTIMEASMOBJS)
	$(LINKER) $(NTKCLINKFLAGS) $^ -o $@

$(NTKCCXXOBJS) : $(OBJDIR)/%.cpp.o : %.cpp
	$(CXX) $(CXXFLAGS) $(NEWTONINCLUDES) -c $< -o $@

$(NTKCASMOBJS) : $(OBJDIR)/%.s.o : %.s
	$(AS) $(ASFLAGS) -c $< -o $@

# Protocol rules

PROTOCXXOBJS := $(OBJDIR)/PDemo.cpp.o
PROTOASMOBJS := $(OBJDIR)/PDemo.impl.s.o
PROTOLINKFLAGS := -EB -shared --gc-sections --no-allow-shlib-undefined --no-export-dynamic -x --no-undefined -Tprotocol.ld --version-script=protocol.map
PROTOPKGNAME := "DemoProtocol:Kallisys"

protocol.pkg : $(OBJDIR)/protocol.elf
	rm -f $@
	ELFtoPKG -o $@ $(PROTOPKGNAME) -protocol $< -autoLoad -autoRemove
	SetFile -t 'pkg ' -c 'pkgX' $@

$(OBJDIR)/protocol.elf : $(PROTOCXXOBJS) $(PROTOASMOBJS) $(RUNTIMECXXOBJS) $(RUNTIMEASMOBJS)
	$(LINKER) $(PROTOLINKFLAGS) $^ -o $@

$(PROTOCXXOBJS) : $(OBJDIR)/%.cpp.o : %.cpp
	$(CXX) $(CXXFLAGS) $(NEWTONINCLUDES) -c $< -o $@

$(PROTOASMOBJS) : $(OBJDIR)/%.s.o : %.s
	$(AS) $(ASFLAGS) -c $< -o $@

