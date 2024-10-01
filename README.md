# One: an experimental programming language and compiler

This is just me trying to craft a working compiler for a simple language.
The direction is unclear -- I have no concrete plans for this project and I am designing
the language as I work through the compiler.

## Building

Currently, onec does not depend on any external libraries, so building is straight forward.
Simply execute the default target using make:

```sh
make
```

This compiles onec and outputs the binary at `build/onec`.

## Running tests

You need Python 3.12 installed in order to run tests. Other versions of Python 3.x might work
but I have only tested with 3.12.

Install dependencies from requirements.txt:

```sh
pip install -r t/requirements.txt
```

Finally, use the `test` target to run the tests:

```sh
make test
```
