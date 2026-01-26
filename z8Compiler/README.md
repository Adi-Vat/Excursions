# Z8 Compiler + Emulator
A compiler for my (toy) 'Z8' assembly language. Inspired (sort of) by z80 ASM.
I've also included an emulator to execute the machine code. The code can be run on an FPGA with a little CPU
made in System Verilog, but that's not included here.

As an example, I've written a Fibonacci sequence algorithm, and compiled it into byte code.
Then I run the byte code in the emulator.  

![](Z8_Compiler_and_Emulator.gif)

## Example code pipeline
The assembly file
```
.fibonacci
    LDD y, 5
    LDD a, 1
    LDD x, 1
    LDD z, 0
.loop
    LDD z, a
    ADC x
    LDD x, z
    OUT a
    DEC y
    JNZ .loop
```
Compiles to a hex file  

```
0200A60005
0200A40001
0200A50001
0200A70000
0100A700A4
0600A50000
0100A500A7
1C00A40000
0B00A60000
1800040000
```
And is executed with a result of
```
2
3
5
8
13
```

## Instruction Set Architecture
- `NOP` No operation
- `LDD` Load directly into register `<REG>` from `<MEMORY/REG>`
- `LDD_I`
- `LDR` Load into register `<REG>` data at memory location in `<MEMORY/REG>`
- `STR` Store into `<MEMORY>` data from `<REG/VALUE>`
- `STR_I`
- `ADC` Add with carry acc and `<REG/VALUE>` store into acc
- `ADC_I`
- `SUB` Subtract acc and `<REG/VALUE>` store into acc
- `SUB_I`
- `INC` Increment `<REG>`
- `DEC` Decrement `<REG>`
- `ROL` Rotate left `<REG>`
- `ROR` Rotate right `<REG>`
- `SLL` Shift left logical `<REG>`
- `SLA` Shift left arithmetic `<REG>`
- `SRL` Shift right logical `<REG>`
- `SRA` Shift right arithmetic `<REG>`
- `PSH` Push `<REG>` to stack
- `POP` Pop stack onto `<REG>`
- `JMP` Jump to `<VAL>`
- `CMP` Compare `<REG>` with acc
- `CMP_I`
- `JZ` Jump to `<VAL>` if zero flag is set
- `JNZ` Jump to `<VAL>` if zero flag is not set
- `JN` Jump to `<VAL>` if negative flag is set
- `SB` Set at `<MEMORY/REG>` bit number `<VAL>` to 1
- `CB` Clear at `<MEMORY/REG>` bit number `<VAL>` to 0
- `OUT` Prints value in `<MEMORY/REG>` to console (emulator only)

A whole instruction is 40 bits long, split into nibbles.  
The instruction breaks down as such:  
'LDD y, 5' becomes
```
[0][1][2][3][4][5][6][7][8][9]  
 0  2  0  0  A  6  0  0  0  5
```
0-1: opcode (2 -> LDD_I)  
2-5: operand A (A6 -> register y)  
6-9: operand B  (5)

## Memory Mapping
The z8 processor can index up to 0xFFFF (65,535) bytes of memory
0000 -> FFFF  
0000 -> 0003 = BANK0  
0004 -> 0009 = BANK1  
00A0 -> 00A1 = LEDRs  
00A2 -> 00A3 = INPUTs  
00A4 -> 00A7 = REGs  
00A8 = FLAGS  
00A9 -> 00B8 = STACK  

## Unique features
All immediate instruction variations are implicit, the user does not need to write them. Instead the compiler takes the context of the second operand to choose the operation type and +1 for an immmediate operation.

Memory addresses are indicated with a `*`  
Hex is indicated with a `$`  
Denary is the default  
Labels are indicated with a `.`  
Comments are indicated with a `;`

The emulator provides a memory dump after execution.

## To do
- Allow inline comments
- Finish implementing instruction set into emulator
- Add logical operations to instruction set
- Fix possible future error where JMP `<VAL>` does not compile correctly; `<VAL>` will be compiled as the text-editor line **not** the machine code line as it should be (and as is done automatically with labels)
