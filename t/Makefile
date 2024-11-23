all::
.PHONY: all

CFLAGS = -g -Werror -Werror -I..
CC = gcc

PYTHON = python3

BUILD_DIR = ../build

export PATH := $(BUILD_DIR):$(PATH)

TEST_FILES += ast
TEST_FILES += typecheck
test:
	$(PYTHON) -m pytest $(TEST_FILES)
.PHONY: test

all:: test

clean:
	rm -rf __pycache__
	rm -rf .pytest_cache
.PHONY: clean
