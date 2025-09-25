SRC := \
	async.c \
	setjmp.s \

DEFAULT_CFLAGS := -D_POSIX_C_SOURCE=200809L -fno-omit-frame-pointer -pipe -march=native -std=c99 -pedantic -Wall -Werror -Wextra
LIBRARIES := -pthread
CFLAGS         ?= -Og -ggdb3
CC             ?= cc
BUILDDIR       ?= out
TARGET         ?= badidea.a
.DEFAULT_GOAL   = $(BUILDDIR)/$(TARGET)

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
$(BUILDDIR)/await: await.c $(BUILDDIR)/$(TARGET)
$(BUILDDIR)/cancel: cancel.c $(BUILDDIR)/$(TARGET)

.PHONY: clean
clean:
	-rm -rf $(BUILDDIR)

