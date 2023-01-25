PROJNAME := zodiac
CXXFLAGS := -std=c++11 -Isrc -I/usr/include/iniparser/ -fno-exceptions -fno-rtti
LIBS := ssl crypto iniparser ssockets
RUN = ./$(RESULTLINK)

# ---------------------

# tbs 1.0.0-RC2

# 1. INTRODUCTION
# This is tbs, the trivial build system, a Makefile for C/C++/asm projects.
# This is public domain; feel free to copy/paste, modify, and share me.
# tbs 1.0.0-RC2 by jlxip, January 25th 2023

# 2. DOCUMENTATION
# tbs is configured via a series of variables at the top of this file.
# If this is your first time using tbs, just remove everything above the line
# at the top of this huge comment, and set the values as you please.
# Variables that you want to unset (those that have a default value), must
# be set to a space. Example: LTO := " "
# Variables that you want to set (those that do not have a default value), may
# be set to any value. Example: ISLIB := yes
# As all options are just variables, they can be overriden at runtime via
# environment ones. For instance:
# LOUD=1 make
# will print all executed commands, which is not the default behavior.

# 2.0. tl;dr
# PROJNAME := your_project
# This is enough for most projects.

# 2.1. PROJECT
# Set your project name in $(PROJNAME). This value will be shown while building.
# If your project is a library:
# - Set $(ISLIB) to a non-empty value
# - If you want to link a shared library (".so"), set $(MAKESHARED)
# - If you want to pack a static library (".a"),  set $(MAKESTATIC)
# - You can set both
# The flag "-s" (silent) is used by default in this Makefile. If you do not
# want this behavior, set $(LOUD).

# 2.2. FILES
# If your project results in a binary ($(ISLIB) is not set), then it will be
# compiled to a file named $(PROJNAME); override this behavior by setting
# $(RESULTLINK), for instance:
# RESULTLINK := $(PROJNAME).exe
# If $(MAKESHARED) is set, $(RESULTLINK) will default to "lib$(PROJNAME).so"
# If $(MAKESTATIC) is set, $(RESULTAR) will be used instead, and it defaults
# to "lib$(PROJNAME).a"
# $(RESULTS) is set for convenience to "$(RESULTLINK) $(RESULTAR)".
# Please keep all your source files inside a directory. The default value is
# "src". Override by setting $(SRCPATH).
# Each file in src, if criteria is met, will be compiled to an object under
# the "obj" directory, which will be created if it doesn't exist. You can
# change its name by setting $(OBJPATH).
# Pattern exclusions are possible. Set $(EXCLUDE) to a space-separated list
# of file path patterns to be ignored when listing the sources. Example:
# EXCLUDE := utils/removeme.cpp utils/debug/*.asm ignore/*

# 2.3. COMPILERS
# tbs will compile all C, C++, and asm sources under $(SRCPATH) unless otherwise
# specified. The language of a file is known by its extension:
#   C   -> $(CEX), defaults to ".c"
#   C++ -> $(CXXEX), defaults to ".cpp .cc"
#   ASM -> $(ASMEX), defaults to ".asm .s"
# Separate extensions with spaces. If you unset one of the above (CEX := " "),
# no files will be compiled using its compiler. This is generally not needed.
# The compiler for each language can be overriden as well:
#   C   -> $(CC), defaults to "cc"
#   C++ -> $(CXX), defaults to "g++"
#   ASM -> $(ASM), defauls to "nasm"

# 2.4. FLAGS
# $(CC) and $(CXX) receive the flags given in $(CFLAGS) and $(CXXFLAGS)
# respectively. These variables are built from others and can be changed:
# - $(WALL), defaults to "-Wall -Wextra -Werror"
# - $(OL), defaults to "-O2"
# - $(LTO), defaults to "-flto"
# The special variable $(ISLIB), if set, will add the flag "-fPIC".
# $(ASM) receives the flags given in $(ASMFLAGS).

# 2.5. LINKER
# The linker is defined in $(LINK); $(CC) is used by default, since it can
# be used trivially to link C and assembly objects. If $(CXX) is used at least
# once, then it turns into the default linker, since $(CC) can't link
# to libstdc++. In some specific situations, such as freestanding environments,
# setting it to "ld" may be desirable.
# Following the ideas expressed in the previous section, its flags are set
# in $(LINKFLAGS), and they can be configured indirectly via variables:
# - $(RELRO), defaults to "-Wl,-z,relro,-z,now"
# - $(LTO), defaults to "-flto"
# The (empty by default) variable $(LIBS) will append "-l" to the beginning
# of each entry, so:
# LIBS := m fuse
# will turn into $(LINKFLAGS) having "-lm -lfuse" somewhere in the middle.
# The special variable $(ISLIB) will add the flag "-shared"

# 2.6. MODES
# Your project is by default in "release" mode. This means that the following
# features will be enabled unless otherwise specified:
# - Link Time Optimization (LTO) for $(CC) and $(CXX), by the variable $(LTO)
#   which, as specified previously, defaults to "-flto".
# - Stripping for all results, enabled by the abscence of $(NO_STRIP), and
#   only executed if found on the system.
# You can compile your project in "debug" mode, which will disable all of the
# above. This is done via the $(DEBUG) variable. However, consider NOT setting
# it at the top of the file, and only via an environment variable.

# 2.7. TARGETS
# tbs will define several targets to be used in the "make" order:
# - all: the default, compiles $(RESULTS)
# - install: runs "all" and then installs the project.
#   "Installing" means running "install" (0755) from $(RESULTS) to $(INS).
#   If the result is an executable, $(INS) defaults to "/usr/bin".
#   If it's a library, it defaults to "/usr/lib".
# - uninstall: removes the files created by "install"
# - clean: removes $(RESULTS) and $(OBJPATH)

# 2.8. EXTENSIONS
# All the targets defined above can be extended via custom ones.
# If $(USE_PREALL) is defined, the target "preall", which is expected to have
# been defined by the programmer at the top, will be invoked BEFORE linking.
# If $(USE_POSTALL) is defined, the target "postall" will be invoked AFTER.
# $(SKIP_ALL) will disable the default implementation for "all", leaving
# only "preall" and "postall" to be ran if enabled and defined.
# This applies to all other targets:
# - $(USE_PREINSTALL), $(USE_POSTINSTALL), and $(SKIP_INSTALL)
# - $(USE_PREUNINSTALL), $(USE_POSTUNINSTALL), and $(SKIP_UNINSTALL)
# - $(USE_PRECLEAN), $(USE_POSTCLEAN), and $(SKIP_CLEAN)
# Hint: if any of these extension targets are defined at the top of the file,
# preceed them with yet another target in the lines of:
# .PHONY: aux
# aux: all
# This way, "make" will still be equivalent to "make all"
# If you want a "run" target, you are free to define it like any other; however,
# as these targets are usually one-liners, you can fill the $(RUN) variable,
# in which case a "run" target with your order will be set up.

# 3. CHANGELOG
# Versions follow SemVer 2.0.0
# - 1.0.0-RC2 (2023-01-25):
#   - New debug/release modes (Section 2.6), with strip!
#   - Simpler handling of results via $(RESULTLINK) and $(RESULTAR) instead of
#     $(RESULTSHARED) and $(RESULTSTATIC)
#   - If $(CXX) is used, it turns into the linker by default
#   - If $(ISLIB), $(RESULTLINK) and $(RESULTAR) now prepend "lib" by default
#   - Support for LTO, and enabled by default in release mode
#   - $(RUN) for one-liner "run" targets
#   - Using $(WALL) and $(OL) for linking too
#   - Discovered "?=" which makes for shorter code
#   - Fixed typos, rewrote confusing statements, and expanded on understanding
#     assumptions
# - 1.0.0-RC1 (2023-01-24): first version



# --- Code starts here ---

# Avoid issues in non-GNU systems
SHELL := /bin/sh

# System discovery: is there a "type" shell built-in?
HAVETYPE := $(shell type echo &> /dev/null; echo $$?)
ifeq ($(HAVETYPE),0)
    HAVETYPE := yes
else
    HAVETYPE := ""
endif

# Loud mode?
ifndef LOUD
    MAKEFLAGS += -s
endif

# Basic constraint: $(PROJNAME) must be set
ifndef PROJNAME
    $(error "tbs: you have not set PROJNAME")
endif

# Basic constraint: if $(ISLIB) is set, at least one MAKES* must be set
ifdef ISLIB
    ifndef MAKESHARED
        ifndef MAKESTATIC
            $(error "tbs: ISLIB set, but neither MAKESHARED nor MAKESTATIC")
        endif
    endif
endif

# Default values for RESULTLINK and RESULTAR
ifdef ISLIB
    ifdef MAKESHARED
        RESULTLINK ?= lib$(PROJNAME).so
    endif
    ifdef MAKESTATIC
        RESULTAR ?= lib$(PROJNAME).a
    endif
else
    RESULTLINK ?= $(PROJNAME)
endif

# This is convenient
RESULTS := $(RESULTLINK) $(RESULTAR)

# Default values for SRCPATH and OBJPATH
SRCPATH ?= src
OBJPATH ?= obj

# Default extensions
CEX ?= .c
CXXEX ?= .cpp .cc
ASMEX ?= .asm .s

# CC and CXX come defined in GNU Makefile
# ASM does not (AS actually does, but defaults to "as" which is uncool)
ASM ?= nasm

# Default values for CC and CXX flags
WALL ?= -Wall -Wextra -Werror
OL ?= -O2
ifndef DEBUG
    LTO ?= -flto
endif

# Set up CFLAGS and CXXFLAGS
CFLAGS += $(WALL) $(OL) $(LTO)
CXXFLAGS += $(WALL) $(OL) $(LTO)
ifdef ISLIB
    CFLAGS += -fPIC
    CXXFLAGS += -fPIC
endif

# Default linker
ifndef LINK
    LINK_IS_DEFAULT := yup
    LINK := $(CC)
endif

# LINKFLAGS
RELRO ?= -Wl,-z,relro,-z,now
LINKFLAGS += $(RELRO) $(WALL) $(OL) $(LTO)

# Construct LIBS if given
ifdef LIBS
    LINKFLAGS += $(LIBS:%=-l%)
endif

# If ISLIB is set, create a shared object
# If MAKESHARED is not set, this doesn't matter since the linker is not used
ifdef ISLIB
    LINKFLAGS += -shared
endif

# Stripping
ifndef DEBUG
    ifndef NO_STRIP
        ifdef HAVETYPE
            STRIP := $(shell type -P strip &> /dev/null; echo $$?)
            ifeq ($(STRIP),0)
                # Alright, all good; paranoia is over
                # Mind the semicolon and the absence of ":="
                STRIP = $(shell type -P strip) $(1);
            else
                STRIP := ""
            endif
        endif
    endif
endif

# Default values for INS
ifndef ISLIB
    INS ?= /usr/bin
else
    INS ?= /usr/lib
endif



# That's all for the flags, let's grab the sources
# This is done via calls to "find", so let's prepare the arguments
# The following JLXIP shenanigans are just to remove the first "-o"
CSRC := JLXIP$(CEX:%=-o -iname '*%')
CXXSRC := JLXIP$(CXXEX:%=-o -iname '*%')
ASMSRC := JLXIP$(ASMEX:%=-o -iname '*%')
# Example: CXXSRC := JLXIP-o -iname '*.cpp' -o -iname '*.cc'

# If one of them is empty, "find" must match no files
ifeq ($(CSRC),JLXIP)
    CSRC := -false
endif
ifeq ($(CXXSRC),JLXIP)
    CXXSRC := -false
endif
ifeq ($(ASMSRC),JLXIP)
    ASMSRC := -false
endif

CSRC := \( $(CSRC:JLXIP-o%=%) \)
CXXSRC := \( $(CXXSRC:JLXIP-o%=%) \)
ASMSRC := \( $(ASMSRC:JLXIP-o%=%) \)
# Example: CXXSRC := ( -iname '*.cpp' -o -iname '*.cc' )

ifdef EXCLUDE
    CSRC := $(CSRC) \( $(EXCLUDE:%=! -path './%') \)
    CXXSRC := $(CXXSRC) \( $(EXCLUDE:%=! -path './%') \)
    ASMSRC := $(ASMSRC) \( $(EXCLUDE:%=! -path './%') \)
endif
# Example: CXXSRC := ( -iname '*.cpp' -o -iname '*.cc' ) ( ! -iname './debug/*' )

# Find!
CSRC := $(shell cd src && find . -type f $(CSRC) | sed 's/\.\///g')
CXXSRC := $(shell cd src && find . -type f $(CXXSRC) | sed 's/\.\///g')
ASMSRC := $(shell cd src && find . -type f $(ASMSRC) | sed 's/\.\///g')
# Example: CXXSRC := main.c utils/solve.c

# Let's build the objects strings now
# Just add ".o" at the end, and move directories
COBJ := $(CSRC:%=$(OBJPATH)/%.o)
CXXOBJ := $(CXXSRC:%=$(OBJPATH)/%.o)
ASMOBJ := $(ASMSRC:%=$(ASMPATH)/%.o)
# Example: CXXOBJ := obj/main.c.o obj/utils/solve.c.o

# Any CXX objects?
ifdef CXXOBJ
    # Yes, is linker set by default?
    ifdef LINK_IS_DEFAULT
        # Yes, change linker to CXX
        LINK := $(CXX)
    endif
endif

# And these are all the objects
OBJS := $(COBJ) $(CXXOBJ) $(ASMOBJ)

# The following is the $(OBJPATH) directory hierarchy
OBJPATHS := $(shell find src -type d)
OBJPATHS := $(OBJPATHS:src%=obj%)
# Example: OBJPATHS := obj obj/utils



# These are for target extensions
.PHONY: preall postall
ifdef USE_PREALL
    USE_PREALL := preall
endif
.PHONY: preinstall postinstall
ifdef USE_PREINSTALL
    USE_PREINSTALL := preinstall
endif
.PHONY: preuninstall postuninstall
ifdef USE_PREUNINSTALL
    USE_PREUNINSTALL := preuninstall
endif
.PHONY: preclean postclean
ifdef USE_PRECLEAN
    USE_PRECLEAN := preclean
endif



# We can finally start with the targets
# Look at this cool macro
POSTMACRO = if [ ! -z "$(USE_POST$(1))" ]; then $(MAKE) post$(2); fi
.PHONY: all
all: $(RESULTS) | $(USE_PREALL)
	$(call POSTMACRO,ALL,all)

$(RESULTLINK): $(OBJS)
	if [ -z "$(SKIP_ALL)" ]; then \
	    echo "[$(PROJNAME)] Linking..."; \
	    $(LINK) $(OBJS) $(LINKFLAGS) -o $@; \
	    $(call STRIP,$@) \
	fi
$(RESULTAR): $(OBJS)
	if [ -z "$(SKIP_ALL)" ]; then \
	    echo "[$(PROJNAME)] Archiving..."; \
	    $(AR) -rcs $@ $(OBJS); \
	    $(call STRIP,$@) \
	fi

# Include the ".d" files (generated below)
-include $(COBJ:%.o=%.d)
-include $(CXXOBJ:%.o=%.d)

# Object creation requires $(OBJPATH) hierarchy
$(OBJS): | $(OBJPATHS)
$(OBJPATHS):
	mkdir -p $@

# Object creation
$(COBJ): $(OBJPATH)/%.o: $(SRCPATH)/%
	echo "[$(PROJNAME)] ===> $<"
	$(CC) -c -o $@ $< $(CFLAGS)
	$(CC) -MM $< -o $(OBJPATH)/$*.d.tmp $(CFLAGS)
	sed -e 's|.*:|$@:|' < $(OBJPATH)/$*.d.tmp > $(OBJPATH)/$*.d
	rm -f $(OBJPATH)/$*.d.tmp

$(CXXOBJ): $(OBJPATH)/%.o: $(SRCPATH)/%
	echo "[$(PROJNAME)] ===> $<"
	$(CXX) -c -o $@ $< $(CXXFLAGS)
	$(CXX) -MM $< -o $(OBJPATH)/$*.d.tmp $(CXXFLAGS)
	sed -e 's|.*:|$@:|' < $(OBJPATH)/$*.d.tmp > $(OBJPATH)/$*.d
	rm -f $(OBJPATH)/$*.d.tmp

$(ASMOBJ): $(OBJPATH)/%.o: $(SRCPATH)/%
	echo "[$(PROJNAME)] ===> $<"
	$(ASM) $< -o $@ $(ASMFLAGS)

# Install
.PHONY: install
install: all | $(USE_PREINSTALL)
	if [ -z "$(SKIP_INSTALL)" ]; then \
	$(foreach result,$(RESULTS),install -Dvm755 $(result) $(INS)/$(result);) \
	fi
	$(call POSTMACRO,INSTALL,install)

# Uninstall
.PHONY: uninstall
uninstall: | $(USE_PREUNINSTALL)
	if [ -z "$(SKIP_UNINSTALL)" ]; then \
	$(foreach result, $(RESULTS), rm -vf $(INS)/$(result);) \
	fi
	$(call POSTMACRO,UNINSTALL,uninstall)

# Cleaning
.PHONY: clean
clean: | $(USE_PRECLEAN)
	if [ -z "$(OBJPATH)" ]; then \
		echo "CAREFUL!"; exit 1; fi
	if [ -z "$(SKIP_CLEAN)" ]; then \
	    rm -rf $(RESULTS) $(OBJPATH)/; fi
	$(call POSTMACRO,CLEAN,clean)

# Extra: run
ifdef RUN
.PHONY: run
run: all
	$(RUN)
endif
