# z8 Processor Core
## Overview
An 8-bit RISC processor written in System Verilog. It's used to run programs output from my [z8 Assembler](../z8Assembler_v2).  
I built it from scratch to learn more about computer architecture, assemblers, and digital logic. It successfuly executes programs including an example Fibonacci series.

[![Watch the video](z8ProcessorCoreThumbnail.png)](https://youtu.be/vuQimqax7PA)

## Specifications
8-bit data width  
40-bit instructions (5-byte format)  
4 general purpose registers indexed R0-R3
256 byte data memory, stack starts at 0xFF and grows downwards  
31 opcodes total, with 18 individual instructions  

## Architecture
![](BlockDiagram.png)
