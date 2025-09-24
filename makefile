SRC := \
	main.c \
	async.c \
	setjmp.s \

DEFAULT_CFLAGS := -D_POSIX_C_SOURCE=200809L -fno-omit-frame-pointer -pipe -march=native -std=c99 -pedantic -Wall -Werror -Wextra
LIBRARIES := -pthread
CFLAGS         ?= -Og -ggdb3
CC             ?= cc
BUILDDIR       ?= out
TARGET         ?= a.out
.DEFAULT_GOAL   = $(BUILDDIR)/$(TARGET)

#OBJ = $(addprefix $(BUILDDIR)/,$(SRC:.c=.o:.s=.o))
OBJ = $(addprefix $(BUILDDIR)/,$(patsubst %.c,%.o,$(patsubst %.s,%.o,$(SRC))))

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $(DEFAULT_CFLAGS) $(CFLAGS) -c $<

$(BUILDDIR)/%.o: %.s | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $(DEFAULT_CFLAGS) $(CFLAGS) -c $<

$(BUILDDIR)/%: | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $(DEFAULT_CFLAGS) $(CFLAGS) $(LIBRARIES) $^

$(BUILDDIR)/%.a: | $(BUILDDIR)
	@mkdir -p $(dir $@)
	ar rcsu $@ $^

$(BUILDDIR)/%.so: | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) -shared -o $@ $(DEFAULT_CFLAGS) $(CFLAGS) $(LIBRARIES) $^

$(BUILDDIR)/$(TARGET): $(OBJ)

.PHONY: clean
clean:
	-rm -rf $(BUILDDIR)

