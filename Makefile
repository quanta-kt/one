all::

CC = gcc
CFLAGS = -g -Wextra -Werror

SRCS += alloc.c
SRCS += ast.c
SRCS += ast_printer.c
SRCS += lex.c
SRCS += main.c
SRCS += parser.c
SRCS += vec.c

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
TEST_FILES += $(TEST_DIR)/test_expr.py
TEST_FILES += $(TEST_DIR)/test_block.py
TEST_FILES += $(TEST_DIR)/test_if.py
TEST_FILES += $(TEST_DIR)/test_var_decl.py
TEST_FILES += $(TEST_DIR)/test_while.py
TEST_FILES += $(TEST_DIR)/test_call.py
TEST_FILES += $(TEST_DIR)/test_fn.py
TEST_FILES += $(TEST_DIR)/test_lambda.py

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
