all::
.PHONY: all

CFLAGS = -g -Werror -Werror -I..
CC = gcc

PYTHON = python3

BUILD_DIR = ../build
TEST_HELPER_BIN = $(BUILD_DIR)/test-helpers

export PATH := $(BUILD_DIR):$(TEST_HELPER_BIN):$(PATH)

TEST_DIRECTORIES += ast
TEST_DIRECTORIES += typecheck
TEST_DIRECTORIES += lex
TEST_DIRECTORIES += integration
TEST_DIRECTORIES += error

TEST_FILES = $(patsubst %, %/*.py, $(TEST_DIRECTORIES))

TEST_TARGETS += $(TEST_DIRECTORIES)
TEST_TARGETS += $(TEST_FILES)

$(TEST_TARGETS):
	$(PYTHON) -m pytest $@
.PHONY: $(TEST_TARGETS)

test:
	$(PYTHON) -m pytest $(TEST_FILES)
.PHONY: test

all:: test

clean:
	rm -rf __pycache__
	rm -rf .pytest_cache
.PHONY: clean
