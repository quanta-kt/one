all::

CC = gcc
CFLAGS = -g -Wextra -Werror

BUILD_DIR = build
TEST_DIRECTORY = t
TEST_HELPER_BIN = $(BUILD_DIR)/test-helpers

OUTPUT = $(BUILD_DIR)/onec

export PATH := $(BUILD_DIR):$(PATH)

SRCS += alloc.c
SRCS += ast.c
SRCS += ast_printer.c
SRCS += lex.c
SRCS += main.c
SRCS += typecheck.c
SRCS += parser.c

HEADERS += alloc.h
HEADERS += ast.h
HEADERS += ast_printer.h
HEADERS += lex.h
HEADERS += parser.h
HEADERS += typecheck.h
HEADERS += vec.h

OBJS = $(patsubst %.c, build/%.o, $(SRCS))

all:: $(OUTPUT)
.PHONY: all

$(OUTPUT): $(OBJS)
	@mkdir -p build
	$(CC) $(CFLAGS) $(OBJS) -o $(OUTPUT)

build/%.o: %.c $(HEADERS)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean: clean-test
	rm -rf $(BUILD_DIR)
.PHONY: clean


test:
	make -C $(TEST_DIRECTORY)
.PHONY: test

clean-test:
	make -C $(TEST_DIRECTORY) clean
.PHONY: clean-test
