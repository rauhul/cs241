OBJS_DIR = .objs

# define all of student executables
EXE1=vector_test
EXE2=document_test
EXES_STUDENT=$(EXE1) $(EXE2)
EXES_PROVIDED=$(EXE1)-reference $(EXE2)-reference

# a list of executables to use in testing
# these should be ordered in the order in which the student will probably
# complete them. If the first set of tests fails, then the second set will never
# be run
TEST_EXES=$(EXE1) $(EXE2)

# list object file dependencies for each
OBJS_PART1=vector.o vector_test.o
OBJS_PART2=document.o vector.o document_test.o

# don't include the vector or document in the objects for the -reference
# versions
OBJS_PART1a=vector_test.o
OBJS_PART2a=document_test.o

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
CFLAGS_DEBUG   = -O0 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O2 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE

# set up linker
LD = clang
LDFLAGS =
LDFLAGS-reference = -Llibs/ -lprovided

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

release: $(EXES_STUDENT) $(EXES_PROVIDED)
debug:   clean $(EXES_STUDENT:%=%-debug) $(EXES_PROVIDED)

.PHONY: test
test: $(OBJS_DIR)/tests_run

# not phony, creates a file to indicate that the tests were run recently
$(OBJS_DIR)/tests_run: $(TEST_EXES:%=%-testoutput)
	@echo "tests ran succefully at" > $@
	@date                           >> $@

.DELETE_ON_ERROR:
%-testoutput: %
	./$(@:%-testoutput=%) > $@

# include dependencies
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
# you will need a pair of exe and exe-debug targets for each exe
$(EXE1)-debug: $(OBJS_PART1:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE1): $(OBJS_PART1:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE1)-reference: $(OBJS_PART1a:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS-reference) -o $@

$(EXE2)-debug: $(OBJS_PART2:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE2): $(OBJS_PART2:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE2)-reference: $(OBJS_PART2a:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS-reference) -o $@

.PHONY: clean
clean:
	rm -rf .objs $(EXES_STUDENT) $(EXES_STUDENT:%=%-debug) $(EXES_PROVIDED) $(TEST_EXES:%=%-testoutput)

