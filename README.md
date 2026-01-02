# CHIP-8 Interpreter

A CHIP-8 Interpreter written in C++ with SDL 3

## Help

Build with cmake, requires the rom path as argument.

Optionally the --original and the --debug flags are available, as well as a speed parameter

--original: Use the original COSMAC-VIP Interpretation of the instructions rather than the modern one;

--debug: Enables debug log to the terminal and single stepping.

--speed: Followed by a number, lets youu control the number of cycles per frame