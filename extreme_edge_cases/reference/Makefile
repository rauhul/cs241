OBJS_DIR = .objs

# define all of student executables
EXE1=camelCaser
EXES_STUDENT=$(EXE1)

# tests are defined a little differently for this assignment, see below

# list object file dependencies for each
OBJS_CAMELCASER=camelCaser.o camelCaser_main.o

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
CFLAGS_DEBUG   = -O0 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O2 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE

# set up linker
LD = clang
LDFLAGS =

# the string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
IS_VM=$(shell hostname | grep "fa16")

ifeq ($(IS_VM),)
$(error This assignment must be compiled on the CS241 VMs)
endif

.PHONY: all
all: release

# build types
.PHONY: release
.PHONY: debug

release: $(EXES_STUDENT)
debug:   clean $(EXES_STUDENT:%=%-debug)


# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)
	@cp $(EXE1)-reference.o $(OBJS_DIR)/$(EXE1).o

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly

$(OBJS_DIR)/%.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
# you will need a pair of exe and exe-debug targets for each exe

$(EXE1): $(OBJS_CAMELCASER:%.o=$(OBJS_DIR)/%.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf .objs/camelCaser_main.* $(EXES_STUDENT)