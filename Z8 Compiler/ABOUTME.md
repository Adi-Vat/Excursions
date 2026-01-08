# Z8 Compiler + Emulator
A compiler for my (toy) 'Z8' assembly language. Inspired (sort of) by z80 ASM.
I've also included an emulator to execute the machine code. The code can actually be executed on an FPGA with a little CPU
made in System Verilog, but that's not included here.

As an example, I've written a Fibonacci sequence algorithm, and compiled it into byte code.
Then I run the byte code in the emulator.  
![](Z8_Compiler_and_Emulator.gif)

## To do
- Allow inline comments
- Finish adding full instruction set
