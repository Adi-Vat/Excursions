# Z8 Assembler (v2)
## Overview
An assembler for my own assembly language, z8.  
Converts an assembly file to a hex file.
The hex file can then be run by my [z8 Processor Core](/z8ProcessorCore/)  

## Syntax
### Instructions
Operations are written as `OPCODE <dest>, <src>`  

Variables are declared with `.var <name> <value>`  

Labels are declared with `<name>:`  

### Addressing modes
Memory `@<address>` or `@<variable name>`  

Register `R<index>`  

Direct `<value>`

### Bases
Denary `<value>`  

Hex `$<value>`  

Binary `&<value>`  

## How to use
### Assemble a source file
To assemble a program, run `z8 <input_file> [-o <output_file>]`  

Arguments:  
    `<input_file>`  Assembly source file  
    `-o <output_file>`   Output file (optional, default: `<input_file>.hex`)  

### Compile the assembler
You need a C compiler. I use MinGW-w64 and compile with   
`gcc asm.c shared.c -o z8.exe`

### Examples
Included in `/examples/` is `fibonacci.s` which calculates the fibonacci sequence for $F_7$  
```asm
LD R0, 5    ; Loop counter
LD R1, 1    ; Previous Fibonacci number
LD R2, 1    ; Current Fibonacci number
LD R3, 0    ; Temporary storage

loop:
    LD R3, R2   ; Save current value
    ADD R2, R1  ; Calculate next Fibonacci: R2 = R1 + R2
    LD R1, R3   ; Shift: previous = old current value

    DEC R0  ; Decrement counter
    JNZ loop ; Continue if counter != 0

; Result: R2 = 13 (0x0D)
```
After execution, `R2` will contain `0x0D`, the 7th Fibonacci number.  

This example demonstrates:  
- Arithmetic operations (ADD)  
- Register-to-register moves (LD)  
- Conditional branching (JNZ)  
- Loop implementation

It compiles to
```bash
0300000005 # LD R0, 5
0300010001 # LD R1, 1
0300020001 # LD R2, 1
0300030000 # LD R3, 0
0200030002 # loop: LD R3, R2
0600020001 # ADD R2, R3
0200010003 # LD R1, R3
1B00000000 # DEC R0
1700040000 # JNZ loop
fe00000000 # HALT
```
(without the comments)

If assembly was unsucessful, an output file will not be created, and all reached errors are output, e.g.,
```
[Line 2] Error: cannot directly access protected memory 0x0000 - 0x0100
[Line 2] Error: value is out of range [-32768, 32767]
[Line 3] Error: trying to access an undeclared variable/label
```
