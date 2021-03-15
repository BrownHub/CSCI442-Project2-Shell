SHELL:=/bin/bash
CC=clang
LIBS:=-lreadline -lhistory
FLAGS_release:=-O2 -flto
FLAGS_debug:=-O0 -ggdb3 -DTEST_BUILD -fsanitize=address
CFLAGS:=-std=gnu17 -Werror -Wall -Wstrict-prototypes -Wmissing-prototypes \
	-Wundef -Wmissing-declarations -Iinclude
OUTDIR:=build
LD:=$(CC)
LDFLAGS:=$(CFLAGS)

# Recursive wildcard function, stackoverflow.com/questions/2483182
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Collect the source files
HEADERS:=$(call rwildcard,include,*.h)
SRCS:=$(call rwildcard,src,*.c)
MAINSRCS:=$(call rwildcard,mains,*.c)
OBJFILES_SRC:=$(patsubst %.c,%.o,$(SRCS))
OBJFILES_SRC_debug:=$(foreach o,$(OBJFILES_SRC),$(OUTDIR)/debug/$(o))
OBJFILES_SRC_release:=$(foreach o,$(OBJFILES_SRC),$(OUTDIR)/release/$(o))
OBJFILES_MAINS:=$(patsubst %.c,%.o,$(MAINSRCS))
OBJFILES_MAINS_debug:=$(foreach o,$(OBJFILES_MAINS),$(OUTDIR)/debug/$(o))
OBJFILES_MAINS_release:=$(foreach o,$(OBJFILES_MAINS),$(OUTDIR)/release/$(o))
BINS:=$(patsubst mains/%.c,%,$(MAINSRCS))
BINS_debug:=$(foreach f,$(BINS),$(f).debug)
BINS_release:=$(foreach f,$(BINS),$(f))
OUTPUTS_debug:=$(OBJFILES_SRC_debug) $(OBJFILES_MAINS_debug) $(BINS_debug) \
	$(CINCLUDES_debug)
OUTPUTS_release:=$(OBJFILES_SRC_release) $(OBJFILES_MAINS_release) \
	$(BINS_release) $(CINCLUDES_release)
OUTPUTS:=$(OUTPUTS_debug) $(OUTPUTS_release)
DIRS:=$(sort $(dir $(OUTPUTS)))

_create_dirs := $(foreach d,$(DIRS),$(shell [ -d $(d) ] || mkdir -p $(d)))

ifeq ($(V),)
cmd = @printf '  %-6s %s\n' $(cmd_$(1)_name) "$(if $(2),$(2),"$@")" ; $(call cmd_$(1),$(2))
else
ifeq ($(V),1)
cmd = $(call cmd_$(1),$(2))
else
cmd = @$(call cmd_$(1),$(2))
endif
endif

get_target = $(word 2,$(subst /, ,$@))
target_flags = $(FLAGS_$(get_target))

DEPFLAGS = -MMD -MP -MF $@.d

cmd_c_to_o_name = CC
cmd_c_to_o = $(CC) $(CFLAGS) $(target_flags) $(DEPFLAGS) -c $< -o $@

cmd_o_to_elf_release_name = LD
cmd_o_to_elf_release = $(LD) $(LDFLAGS) $(FLAGS_release) $^ $(LIBS) -o $@

cmd_o_to_elf_debug_name = LD
cmd_o_to_elf_debug = $(LD) $(LDFLAGS) $(FLAGS_debug) $^ $(LIBS) -o $@

cmd_clean_name = CLEAN
cmd_clean = rm -rf $(1)

cmd_gdb_name = GDB
cmd_gdb = $(MAKE) $(1) && gdb $(1)

cmd_run_name = RUN
cmd_run = $(MAKE) $(1) && ./$(1)

.SECONDARY:
.PHONY: all
all: $(BINS_debug) $(BINS_release)

-include $(call rwildcard,$(OUTDIR),*.d)

%.debug: $(OUTDIR)/debug/mains/%.o $(OBJFILES_SRC_debug)
	$(call cmd,o_to_elf_debug)
%: $(OUTDIR)/release/mains/%.o $(OBJFILES_SRC_release)
	$(call cmd,o_to_elf_release)

$(OUTDIR)/debug/%.o: %.c
	$(call cmd,c_to_o)
$(OUTDIR)/release/%.o: %.c
	$(call cmd,c_to_o)

.PHONY: clean
clean:
	$(call cmd,clean,$(OUTDIR) $(BINS_debug) $(BINS_release))
