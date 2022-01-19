# rz
Building rz36-3 compiler on Linux and macOS
## Requirements
- c compiler (tested with gcc and clang)
- make
## Building the program
1. Run this command `make`
2. `rz36`, `as21` and `sim21` binary will be available
- If `cc` command is not found, edit `CC = cc` Makefile to `CC = YOUR-C-COMPILER`
## Using the program
Compiling `test/double.txt`
1. Compile `rz36 test/double.txt > double-s.txt`
2. Assemble `as21 double-s.txt`
3. Run in simulator `sim21 double-s.obj`
