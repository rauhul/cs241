# see ../../deploy.make.common for most of the instructions.

OBJS_DIR = .objs

# define simulator executables
EXE_SIMULATOR=simulator

# list objec file dependencies for each
OBJS_SIMULATOR=$(EXE_SIMULATOR).o
OBJS_SCHEDULER=libscheduler.o
OBJS_ALL=$(OBJS_SIMULATOR) $(OBJS_SCHEDULER)

# set up compiler
CC = clang
INCLUDES=-I./libs/
WARNINGS = -Wall -Wextra -Wno-unused -Wno-unused-parameter
CFLAGS_DEBUG = -O0 $(INCLUDES) $(WARNINGS) -g -std=c99 -c  -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O2 $(INCLUDES) $(WARNINGS) -std=c99 -c  -D_GNU_SOURCE

# set up linker
LD = clang
LDFLAGS = -Llibs/ -lprovided

# The string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
#IS_VM=$(shell hostname | grep "fa16")

#ifeq ($(IS_VM),)
#$(error This assignment must be compiled on the CS241 VMs)
#endif

.PHONY: all
all: release

.PHONY: debug
.PHONY: release

release: $(EXE_SIMULATOR)
debug:   $(EXE_SIMULATOR:%=%-debug)

#include dependencies
-include $(OBJS_DIR)/*.d
$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
# you will need exe and exe-debug
$(EXE_SIMULATOR): $(OBJS_ALL:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_SIMULATOR)-debug: $(OBJS_ALL:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@


.PHONY : clean
clean:
	rm -rf .objs $(EXE_SIMULATOR) $(EXE_SIMULATOR:%=%-debug)
