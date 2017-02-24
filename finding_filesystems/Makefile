OBJS_DIR = .objs

# define all the student executables
MINIX_EXE=minixfs
EXES_STUDENT=$(MINIX_EXE)
# list object file dependencies for each
OBJS_MINIX=format.o fs.o fs_main.o fs_provided.o fs_utils.o
FS_FILES=fs.c fs_provided.c fs_utils.c fs_main.c

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-result
CFLAGS_DEBUG   = -O0 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE
CFLAGS_RELEASE = -O0 $(WARNINGS) -std=c99 -c -MMD -MP -D_GNU_SOURCE

# set up linker
LD = clang
LDFLAGS = -lrt

# the string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
# IS_VM=$(shell hostname | grep "fa16")
#
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

release: $(EXES_STUDENT)
debug:   clean $(EXES_STUDENT:%=%-debug)

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@


# exes

$(MINIX_EXE): $(OBJS_MINIX:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(MINIX_EXE)-debug: $(OBJS_MINIX:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	-rm -rf .objs $(EXES_STUDENT) $(EXES_STUDENT:%=%-debug)

.PHONY: reset
reset:
	-rm -rf test_fs/
	make fs-get

.PHONY: fs-get
testfs:
	svn export https://subversion.ews.illinois.edu/svn/fa16-cs241/_shared/finding_filesystems/test.fs
