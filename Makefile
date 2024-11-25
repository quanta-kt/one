all::

CC = gcc
CFLAGS = -g -Wextra -Werror

BUILD_DIR = build
TEST_DIRECTORY = t
TEST_HELPER_BIN = $(BUILD_DIR)/test-helpers

OUTPUT = $(BUILD_DIR)/onec

export PATH := $(BUILD_DIR):$(PATH)

OBJ += alloc.o
OBJ += ast.o
OBJ += ast_printer.o
OBJ += lex.o
OBJ += main.o
OBJ += typecheck.o
OBJ += parser.o
OBJ := $(addprefix $(BUILD_DIR)/,$(OBJ))

HEADERS += alloc.h
HEADERS += ast.h
HEADERS += ast_printer.h
HEADERS += lex.h
HEADERS += parser.h
HEADERS += typecheck.h
HEADERS += vec.h

all:: $(OUTPUT)
.PHONY: all

$(OUTPUT): $(OBJ)
	@mkdir -p build
	$(CC) $(CFLAGS) $(OBJ) -o $(OUTPUT)

$(OBJ): build/%.o: %.c $(HEADERS)
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
