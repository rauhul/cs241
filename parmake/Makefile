# use gcc with -fsanitize=thread bc clang on student vms (3.4) has buggy thread
# sanitizer
# this means we also need -ltsan in the link step. libtsan must be installed
# (sudo yum install libtsan on student vms)

OBJS_DIR = .objs

# define all the student executables
EXE_PARMAKE=parmake
EXES_STUDENT=$(EXE_PARMAKE)

# list object file dependencies for each
OBJS_PARMAKE=parmake.o parser.o rule.o parmake_main.o common_vector.o

# set up compiler
CC = gcc
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
INC=-I./includes/
CFLAGS_DEBUG   = -O0 $(WARNINGS) $(INC) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -pthread -DDEBUG
CFLAGS_RELEASE = -O2 $(WARNINGS) $(INC) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -pthread

# tsan needs some funky flags
CFLAGS_TSAN    = $(CFLAGS_DEBUG)
CFLAGS_TSAN    += -fsanitize=thread -DSANITIZE_THREADS

# set up linker
LD = gcc
LDFLAGS = -pthread -Llibs/ -lprovided
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

release: $(EXES_STUDENT)
debug:   clean $(EXES_STUDENT:%=%-debug)
tsan:    clean $(EXES_STUDENT:%=%-tsan)

.PHONY: test
test: $(TESTERS)

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
$(EXE_PARMAKE): $(OBJS_PARMAKE:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_PARMAKE)-debug: $(OBJS_PARMAKE:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_PARMAKE)-tsan: $(OBJS_PARMAKE:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LDFLAGS_TSAN) -o $@

# tester exes
$(PARSER_TESTER): $(PARSER_TESTER_DEPS:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all $@

.PHONY: clean
clean:
	-rm -rf .objs $(EXES_STUDENT) $(EXES_STUDENT:%=%-tsan) $(EXES_STUDENT:%=%-debug) $(EXES_PROVIDED) $(EXES_OPTIONAL)
