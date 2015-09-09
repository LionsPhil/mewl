# Phil's Universal Makefile; configured for M.E.W.L.
# BUILD CONFIGURATION ---------------------------------------------------------
  BINARY = mewl
 VERSION = 0.1
 SCRATCH = /tmp/$(BINARY)-scratch/
DISTFILE = $(BINARY)-$(VERSION).zip
 DISTMSG = "M.E.W.L. $(VERSION)"
# Platform can be one of 'posix', 'win', and hopefully one day 'wii'
PLATFORM = posix
# User interface can currently only be 'Sprite'
USERINTF = Sprite

# Expects GNU-flavoured tools
# Because we need C++ linkage, this will use CPPC to compile/link C sources!
      AS = nasm
    CPPC = g++
      LD = g++
     ZIP = zip
   UNZIP = unzip
      SH = sh
    ECHO = echo
  PRINTF = printf
      RM = rm
   MKDIR = mkdir
      CP = cp
      MV = mv
   EGREP = egrep
INKSCAPE = inkscape
     GDB = gdb

# Extra flags to control build type
# Debugging:
  CFLAGSEX = -g -O -DFPS_COUNTER
CPPFLAGSEX = $(CFLAGSEX)
 LDFLAGSEX =
# Release:
#  CFLAGSEX = -O3 -DNDEBUG
#CPPFLAGSEX = $(CFLAGSEX)
# LDFLAGSEX =

# SOURCES ---------------------------------------------------------------------
# Specify source file names here 
# They should be uniquely named (after stripping extensions)
# ASOURCES end in .S, CSOURCES in .c, CPPSOURCES in .cpp and HEADERS in .h
  ASOURCES = 
  CSOURCES = 
CPPSOURCES = controller.cpp difficulty.cpp game.cpp gamelogic.cpp gamesetup.cpp\
             species.cpp resources.cpp util.cpp \
             main.cpp platform_$(PLATFORM).cpp
# Headers can be called whatever you want
   HEADERS = controller.hpp difficulty.hpp game.hpp gamelogic.hpp gamesetup.hpp\
             species.hpp resources.hpp util.hpp \
             factory.hpp platform.hpp ui.hpp

# User interface files and flags
ifeq ($(USERINTF),Sprite)
    CPPSOURCES += ui_sprite.cpp ui_sprite_pointer.cpp ui_sprite_title.cpp \
                  ui_sprite_setup.cpp
    HEADERS    += ui_sprite.hpp ui_sprite_pointer.hpp
    LDFLAGSEX  += -lSDL_image -lSDL_mixer -lSDL_ttf
endif

# SVGs from which we autogenerate PNGs
# (We don't actually catch the dervied PNGs with make clean)
SVGPNGS = pointer-centre.svg pointer-north.svg pointer-northeast.svg

# Anything else you want put in the distributed version
# (Include all platform files here; duplication doesn't matter)
# FIXME We won't distribute USERINTFs other than the currently-set one
 EXTRADIST = $(SVGPNGS) Makefile INSTALL VERSION LICENSE \
             platform_posix.cpp platform_win.cpp

# PREAMBLE / AUTOCONFIGURATION / DERIVED --------------------------------------
OBJECTS = $(ASOURCES:%.S=%.o) $(CSOURCES:%.c=%.o) $(CPPSOURCES:%.cpp=%.o)
NOTOBJECTS = $(filter-out %.o, $(OBJECTS))
ifneq ($(NOTOBJECTS),)
    $(error OBJECTS contains non-object(s) $(NOTOBJECTS))
endif
SOURCES = $(ASOURCES) $(CSOURCES) $(CPPSOURCES)

# All files which are sources, /including/ non-compiled ones (e.g. headers)
ALLSOURCESMANU = $(SOURCES) $(HEADERS)

# Compiler warning flags
WARNFLAGS = -Werror -W -Wall -Wpointer-arith -Wcast-align -Wwrite-strings \
            -Wno-unused-parameter -Wuninitialized -Wno-unknown-pragmas
CWFLAGS   = $(WARNFLAGS) -Wcast-qual -Wmissing-declarations \
            -Wnested-externs -Wredundant-decls -Wundef \
            -Wstrict-prototypes -Wmissing-prototypes
CPPWFLAGS = $(WARNFLAGS) -Wno-deprecated
# Need no-depricated due to hash_map getting bitchy in recent GCC.

# Tool flags
# Don't make CXXFLAGS include CFLAGS or it'll get duplicate CFLAGSEX
# Can't have -pedantic for CPPFLAGS due to errors in Box2D's enumerations.
AFLAGS    = -f elf
#CFLAGS    = $(CWFLAGS) -std=c99 -pedantic `sdl-config --cflags` $(CFLAGSEX)
CPPFLAGS  = $(CPPWFLAGS) -std=c++98 -DVERSION='"$(VERSION)"' \
            -DPLATFORM$(PLATFORM) -DUSERINTF='"$(USERINTF)"' \
            `sdl-config --cflags`   $(CPPFLAGSEX)
LDFLAGS   = `sdl-config --libs` -lm $(LDFLAGSEX)

EXTRACDEPS = Makefile $(HEADERS)

# MAKEFILE METADATA AND MISCELLANY --------------------------------------------
# Vpath is a colon separated list of source directories
VPATH = src:svg

# Gratuitous colours. While they make your build process look like an
# '80s disco, they also make it easy to see what it's doing at-a-glance.
RED=\033[31;1m
YELLOW=\033[33;1m
GREEN=\033[32;1m
BLUE=\033[34;1m
MAGENTA=\033[35;1m
CYAN=\033[36;1m
WHITE=\033[0m
RV=\033[7m
COLUMN2 = \033[40G

# Phony targets - these produce no output files (and are not files themselves)
.PHONY: all clean dist disttest work env info run

# Cygwin handling =============================================================
# Autodetect a Cygwin enviroment. make imports enviroment variables, and
# modern Windows tends to have one called "OS", value "Windows_NT".
ifeq ($(OS), Windows_NT)
	ifeq ($(AS), nasm)
		# The Windows version of nasm has a different name
		AS=nasmw.exe
	endif
	# "+=" adds to a variable
	CFLAGS+=-DCYGWIN
	CPPFLAGS+=-DCYGWIN
	# Cygwin terminals don't support ANSI codes
	RED=
	YELLOW=
	GREEN=
	BLUE=
	MAGENTA=
	CYAN=
	WHITE=
	RV=
	COLUMN2=
else
	# Assume Linux.
	CFLAGS+=-DLINUX
	CPPFLAGS+=-DLINUX
endif

# RULES =======================================================================
# $^ is a space-seperated list of all deps, with duplicates removed
# $@ is the current target; $< the dep for that target
# Placing the '@' symbol before a command stops make from echoing the command 
# that it is about to execute. Without it, compiler warnings can get lost under 
# a fair amount of noise.

# This is the default target
all: $(BINARY) $(SVGPNGS:%.svg=data/%.png)

$(BINARY): $(OBJECTS)
	@$(PRINTF) "$(BLUE)--- $(RV)LINKING   $(WHITE) $@\n"
	@$(LD) -o $@ $^ $(LDFLAGS)
	@$(PRINTF) "$(BLUE)$(RV)***$(WHITE) $(BINARY) built\n"

# Pattern rules for creating intermediate objects from sources
%.o : %.c   $(EXTRACDEPS)
	@$(PRINTF) "$(GREEN)--- $(RV)COMPILING $(WHITE) $<\n"
	@$(CPPC) -o $@ -c $(CFLAGS)   $<

%.o : %.cpp $(EXTRACDEPS)
	@$(PRINTF) "$(GREEN)--- $(RV)COMPILING $(WHITE) $<\n"
	@$(CPPC) -o $@ -c $(CPPFLAGS) $<

%.o : %.S
	@$(PRINTF) "$(YELLOW)--- $(RV)ASSEMBLING$(WHITE) $<\n"
	@$(AS) $(AFLAGS) $<

data/%.png: %.svg
	@$(PRINTF) "$(YELLOW)--- $(RV)RENDERING $(WHITE) $<\n"
	@$(INKSCAPE) -z -f $< -C -e $@

# A 'clean' target is handy to zap the intermediate object files
# Typing "make clean" asks make to try to make a "clean", so it follows
# this rule. (Because "clean" is in ".PHONY", make will ignore any file
# which is actually called "clean".)
clean:
	@$(PRINTF) "$(RED)--- $(RV)CLEANING  $(WHITE)\n"
	@$(RM) -fv  $(OBJECTS)
	@$(RM) -frv $(SCRATCH)
	@$(RM) -fv $(BINARY) $(DISTFILE) $(DEFFILE)
	@$(PRINTF) "$(RED)$(RV)***$(WHITE) Cleansed\n"

# Create distributable archive
dist: $(DISTFILE)
$(DISTFILE): $(SOURCES) $(HEADERS) $(EXTRADIST)
	@$(PRINTF) "$(MAGENTA)--- $(RV)BUNDLING  $(WHITE) $(DISTFILE)\n"
	@$(ECHO) $(DISTMSG) | $(ZIP) -z -9 $(DISTFILE) $^
	@$(PRINTF) "$(MAGENTA)$(RV)***$(WHITE) $(DISTFILE) created\n"

# Test distributable
disttest: $(DISTFILE)
	@$(PRINTF) "$(CYAN)--- $(RV)TESTING   $(WHITE) $(DISTFILE)\n"
	@$(MKDIR) -p $(SCRATCH)
	@$(CP) $(DISTFILE) $(SCRATCH)
	@$(ECHO) "cd $(SCRATCH); $(UNZIP) $(DISTFILE); make" | $(SH)
	@$(PRINTF) "$(CYAN)$(RV)***$(WHITE) Distributable tested\n"

# To `make work' is to find things to do ;)
work: $(SOURCES) $(HEADERS)
	@$(PRINTF) "$(RV)***$(WHITE) Outstanding tasks:\n"
	@$(EGREP) '(\\todo|TODO|FIXME)' --color=auto $^ || true

# Build environment information
env:
	@$(ECHO) "Assembler        : $(AS)"
	@$(ECHO) "C++ compiler     : $(CPPC)"
	@$(ECHO) "Linker           : $(LD)"
	@$(ECHO) "Assembler flags  : $(AFLAGS)"
	@$(ECHO) "C compile flags  : $(CFLAGS)"
	@$(ECHO) "C++ compile flags: $(CPPFLAGS)"
	@$(ECHO) "Linker flags     : $(LDFLAGS)"

# Complete Makefile information
info: env
	@$(ECHO) "Assembler sources: $(ASOURCES)"
	@$(ECHO) "C sources        : $(CSOURCES)"
	@$(ECHO) "C++ sources      : $(CPPSOURCES)"
	@$(ECHO) "Objects          : $(OBJECTS)"

run: all
	@$(PRINTF) "$(WHITE)--- $(RV)EXECUTING $(WHITE) $(BINARY)\n"
	@./$(BINARY)

debug: all
	@$(PRINTF) "$(WHITE)--- $(RV)DEBUGGING $(WHITE) $(BINARY)\n"
	@$(GDB) -ex run ./$(BINARY)

