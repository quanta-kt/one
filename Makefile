all::

CC = gcc
CFLAGS = -g -Wextra -Werror

SRCS += alloc.c
SRCS += ast.c
SRCS += ast_printer.c
SRCS += lex.c
SRCS += main.c
SRCS += parser.c

HEADERS += alloc.h
HEADERS += ast.h
HEADERS += ast_printer.h
HEADERS += lex.h
HEADERS += parser.h
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


TEST_DIR = t
PYTHON = python3

# Make lib.py available for import
export PYTHONPATH = t

TEST_FILES += $(TEST_DIR)/ast

test: $(OUTPUT)
	$(PYTHON) -m pytest $(TEST_FILES)
.PHONY: test

clean-pyc:
	rm -rf __pycache__
	rm -rf .pytest_cache
.PHONY: clean-pyc

clean: clean-pyc
	rm -rf $(BUILD_DIR)
.PHONY: clean
