all::

CC = gcc
CFLAGS = -g -Wextra -Werror

BUILD_DIR = build
TEST_DIRECTORY = t
TEST_HELPER_BIN = $(BUILD_DIR)/test-helpers

OUTPUT = $(BUILD_DIR)/onec

export PATH := $(BUILD_DIR):$(PATH)

LIB_OBJ += alloc.o
LIB_OBJ += ast.o
LIB_OBJ += ast_printer.o
LIB_OBJ += lex.o
LIB_OBJ += typecheck.o
LIB_OBJ += parser.o
LIB_OBJ := $(addprefix $(BUILD_DIR)/,$(LIB_OBJ))

LIB_HEADERS += alloc.h
LIB_HEADERS += ast.h
LIB_HEADERS += ast_printer.h
LIB_HEADERS += lex.h
LIB_HEADERS += parser.h
LIB_HEADERS += typecheck.h
LIB_HEADERS += vec.h

ONEC_OBJ += $(BUILD_DIR)/main.o
ONEC_OBJ += $(LIB_OBJ)

all:: $(OUTPUT)
.PHONY: all

$(OUTPUT): $(ONEC_OBJ)
	@mkdir -p build
	$(CC) $(CFLAGS) $(ONEC_OBJ) -o $(OUTPUT)

$(ONEC_OBJ): build/%.o: %.c $(LIB_HEADERS)
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
