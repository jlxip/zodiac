#MAKEFLAGS += -s
PROJNAME := zodiac
RESULT := zodiac

LIBS := ssl crypto iniparser

SHELL := /bin/bash
SRCPATH := src
OBJPATH := obj

CXX := g++
INCLUDES := -Isrc

CXXFLAGS_BASE := -std=c++11 -O2
CXXFLAGS_WARN := -Wall -Wextra -Werror
CXXFLAGS_EXCLUDE := -fno-exceptions -fno-rtti
CXXFLAGS := $(INCLUDES) $(CXXFLAGS_BASE) $(CXXFLAGS_WARN) $(CXXFLAGS_EXCLUDE)

LINKER := $(CXX)
LINKER_FLAGS := -Wl,-z,relro,-z,now
LINKER_FLAGS_END := $(patsubst %,-l%,$(LIBS))

# --- OBJS ---
OBJPATHS := $(shell cd src && find . -type d | xargs -I {} echo "$(OBJPATH)/"{})
OBJS := $(shell cd src && find . -type f -iname '*.cpp' | sed 's/\.\///g' | sed 's/\.cpp/\.o/g' | xargs -I {} echo "$(OBJPATH)/"{})

.PHONY: all clean
all: $(RESULT)
	@

run: all
	@./$(RESULT)

$(RESULT): $(OBJS)
	@echo "[$(PROJNAME)] Linking..."
	@$(LINKER) $(LINKER_FLAGS) $(OBJS) $(LINKER_FLAGS_END) -o $@
	@if [[ ! -v DEBUG ]]; then \
		echo "[$(PROJNAME)] Stripping..."; \
		strip $(RESULT); \
	fi

-include $(CXX_OBJS:.o=.o.d)

$(OBJS): $(OBJPATH)/%.o: $(SRCPATH)/%.cpp | $(OBJPATHS)
	@echo "[$(PROJNAME)] ===> $<"
	@$(CXX) -c -o $@ $< $(CXXFLAGS)
	@$(CXX) -MM $< -o $@.d.tmp $(CXXFLAGS)
	@sed -e 's|.*:|$@:|' < $@.d.tmp > $@.d
	@rm -f $@.d.tmp

$(OBJPATHS): $(OBJPATH)/%: $(SRCPATH)/%
	@mkdir -p $(OBJPATH)/$*

clean:
	@rm -rf $(RESULT) $(OBJPATH)/
