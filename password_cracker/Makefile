# use gcc with -fsanitize=thread bc clang on student vms (3.4) has buggy thread
# sanitizer
# this means we also need -ltsan in the link step. libtsan must be installed
# (sudo yum install libtsan on student vms)

OBJS_DIR = .objs

# define all the student executables
EXE_CRACKER1=cracker1
EXE_CRACKER2=cracker2
EXES_STUDENT=$(EXE_CRACKER1) $(EXE_CRACKER2)

# list object file dependencies for each
OBJS_ALL=libs/utils.o libs/thread_status.o libs/format.o
OBJS_CRACKER1=$(OBJS_ALL) $(EXE_CRACKER1)_main.o $(EXE_CRACKER1).o
OBJS_CRACKER2=$(OBJS_ALL) $(EXE_CRACKER2)_main.o $(EXE_CRACKER2).o

# define the provided code exes and deps
EXE_EX=create_examples
OBJS_EX=tools/$(EXE_EX).o
EXES_PROVIDED=$(EXE_EX)

# optional things
EXE_TIMING=timing
OBJS_TIMING=tools/$(EXE_TIMING).o libs/utils.o

EXES_OPTIONAL=$(EXE_TIMING)

# set up compiler
CC = gcc
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
INC=-Ilibs/
CFLAGS_DEBUG   = -O0 $(WARNINGS) $(INC) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -pthread -DDEBUG
CFLAGS_RELEASE = -O2 $(WARNINGS) $(INC) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -pthread

# tsan needs some funky flags
CFLAGS_TSAN    = $(CFLAGS_DEBUG)
CFLAGS_TSAN    += -fsanitize=thread -DSANITIZE_THREADS -fPIC

# set up linker
LD = gcc
LDFLAGS = -lcrypt -lrt -pthread -Llibs/ -lprovided
LDFLAGS_TSAN = $(LDFLAGS) -ltsan

# the string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
# IS_VM=$(shell hostname | grep "fa16")

# ifeq ($(IS_VM),)
# $(error This assignment must be compiled on the CS241 VMs)
# endif

.PHONY: all
all: release

# build types
# run clean before building debug so that all of the release executables
# disappear
.PHONY: debug
.PHONY: release
.PHONY: tsan

release: $(EXES_STUDENT) $(EXES_PROVIDED)
debug:   clean $(EXES_STUDENT:%=%-debug) $(EXES_PROVIDED)
tsan:    clean $(EXES_STUDENT:%=%-tsan) $(EXES_PROVIDED)

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-tsan.o: %.c | $(OBJS_DIR)
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS_TSAN) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
# you will need a triple of exe and exe-debug and exe-tsan for each exe (other
# than provided exes)
$(EXE_CRACKER0): $(OBJS_CRACKER0:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CRACKER0)-debug: $(OBJS_CRACKER0:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CRACKER0)-tsan: $(OBJS_CRACKER0:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LDFLAGS_TSAN) -o $@

$(EXE_CRACKER1): $(OBJS_CRACKER1:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CRACKER1)-debug: $(OBJS_CRACKER1:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CRACKER1)-tsan: $(OBJS_CRACKER1:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LDFLAGS_TSAN) -o $@

$(EXE_CRACKER2): $(OBJS_CRACKER2:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CRACKER2)-debug: $(OBJS_CRACKER2:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CRACKER2)-tsan: $(OBJS_CRACKER2:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LDFLAGS_TSAN) -o $@

$(EXE_EX): $(OBJS_EX:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_TIMING): $(OBJS_TIMING:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	-rm -rf .objs $(EXES_STUDENT) $(EXES_STUDENT:%=%-tsan) $(EXES_STUDENT:%=%-debug) $(EXES_PROVIDED) $(EXES_OPTIONAL)
