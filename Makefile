all::

CC = gcc
CFLAGS = -g -Wextra -Werror

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

BUILD_DIR = build
OUTPUT = $(BUILD_DIR)/onec

export PATH := $(BUILD_DIR):$(PATH)

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


TEST_DIRECTORY = t/
test:
	make -C $(TEST_DIRECTORY)
.PHONY: test

clean-test:
	make -C $(TEST_DIRECTORY) clean
.PHONY: clean-test
