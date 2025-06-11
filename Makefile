first::

CC = gcc
CFLAGS = -g -Wextra -Werror -Isrc/

SRC_DIR = src
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
LIB_HEADERS := $(addprefix $(SRC_DIR)/,$(LIB_HEADERS))

ONEC_OBJ += $(BUILD_DIR)/main.o
ONEC_OBJ += $(LIB_OBJ)

first:: $(OUTPUT)
.PHONY: first

$(OUTPUT): $(ONEC_OBJ)
	@mkdir -p build
	$(CC) $(CFLAGS) $(ONEC_OBJ) -o $(OUTPUT)

$(ONEC_OBJ): build/%.o: $(SRC_DIR)/%.c $(LIB_HEADERS)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean: clean-test
	rm -rf $(BUILD_DIR)
.PHONY: clean


TEST_HELPER_OBJS += code2token-list.o
TEST_HELPER_OBJS += code2sexpr.o
TEST_HELPER_OBJS := $(addprefix $(TEST_HELPER_BIN)/,$(TEST_HELPER_OBJS))

$(TEST_HELPER_OBJS): $(TEST_HELPER_BIN)/%.o: \
	$(TEST_DIRECTORY)/helpers/%.c \
	$(LIB_OBJ) \
	$(LIB_OBJ) $(LIB_HEADER)

	@mkdir -p $(TEST_HELPER_BIN)
	$(CC) $(CFLAGS) -c $< -o $@

TEST_HELPER_PROGRAMS = $(TEST_HELPER_OBJS:.o=)

$(TEST_HELPER_PROGRAMS): %: %.o
	$(CC) $(CFLAGS) $< $(LIB_OBJ) -o $@

test: $(OUTPUT) $(TEST_HELPER_PROGRAMS)
	make -C $(TEST_DIRECTORY)
.PHONY: test

clean-test:
	make -C $(TEST_DIRECTORY) clean
.PHONY: clean-test
