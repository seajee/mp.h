# mp.h

A single header library for math expression parsing.

## Quick start

To build and run the examples provided run the following

```bash
$ make
```

## Usage

The library consists on two base components: the tokenizer and the parser.

After these two have processed the input expression an parse tree will be be
created. For evaluating this parse tree, an interpreter and a basic virtual
machine have been implemented in this library.

In the simplified API of the library the compilation approach for the virtual
machine is used by default since it is preferred for the computational speed
advantage.

Examples can be found in the examples folder. A simple REPL and example of the
usage of the simplified API can be found in the root directory of the
repository.
