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
```bash
gcc asm.c shared.c -o z8.exe
```

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

## Implementation details
It is a two-pass assembler.  
### First Pass
Collect labels/variables and store them in a lookup table

### Second Pass
Parse instructions into machine code, replacing labels/variables with their absolute addresses
The assembler decides addressing mode of an instruction from context. Each instruction maps onto multiple opcodes.  
When writing code, the parent instruction is used. `0xFF` is an invalid opcode.

### Instruction set
| Opcode | Memory | Register | Immediate |  
| --- | --- | --- | --- |  
| NOP | 0x00 | 0x00 | 0x00 |  
| LD | 0x01 | 0x02 | 0x03 |  
| STR | 0xFF | 0x04 | 0x05 |  
| ADD | 0xFF | 0x06 | 0x07 |  
| SUB | 0xFF | 0x08 | 0x09 |  
| AND | 0xFF | 0x0A | 0x0B |  
| OR | 0xFF | 0x0C | 0x0D|  
| XOR | 0xFF | 0x0E | 0x0F|  
| CMP | 0xFF | 0x10 | 0x11 |  
| JMP | 0xFF | 0x12 | 0x13 |  
| JZ | 0xFF | 0x14 | 0x15 |  
| JNZ | 0xFF | 0x16 | 0x17 |  
| JN | 0xFF | 0x18 | 0x19 |  
| INC | 0xFF | 0x1A | 0xFF |  
| DEC | 0xFF | 0x1B | 0xFF |  
| PSH | 0xFF | 0x1C | 0x1D |  
| POP | 0x1E | 0x1E | 0x1E |  
| HALT | 0xFE | 0xFE | 0xFE |  

Each instruction is 5 bytes  
- Byte 0: Opcpde
- Bytes 1-2: Destination
- Bytes 3-4: Source

Example:  
`ADD R2, 5 -> 06 0002 0005`

### Full user ISA
#### ADD reg, reg/imm
&rarr; `0x06 ADR` (add register)  
&rarr; `0x07 ADD` (add direct)  
Adds two numbers and stores the result in the src.  

#### AND reg, reg/imm
&rarr; `0x0A ANR` (and register)  
&rarr; `0x0B AND` (and direct)  
Performs the logical and & operator on two numbers and stores the result in the src.

#### CMP reg, reg/imm
&rarr; `0x10 CPR` (compare register)  
&rarr; `0x11 CPD` (compare direct)  
Subtracts src from dest on two numbers and discards the result.

#### DEC reg
&rarr; `0x1B DEC` (decrement register)  
Decrements the value in a register and stores it back in the same register.

#### HALT 
&rarr; `0xFE HALT` (halt program)  
Stops the program from executing any further.

#### INC reg
&rarr; `0x1A INC` (incerment register)  
Increments the value in a register and stores it back in the same register.

#### JMP reg/imm
&rarr; `0x12 JPR` (jump to value in register)  
&rarr; `0x13 JPD` (jump to direct)  
Jumps to a specific instruction line/label.  
*! warning - jumping to a number will not jump to the instruction, need to fix.*

#### JN reg/imm
&rarr; `0x18 JNR` (jump to value in register)  
&rarr; `0x19 JND` (jump to direct)  
Jumps to a specific instruction line/label if the negative flag is set.  
*! warning - jumping to a number will not jump to the instruction, need to fix.*

#### JNZ reg/imm
&rarr; `0x18 JNZR` (jump to value in register)  
&rarr; `0x19 JNZD` (jump to direct)  
Jumps to a specific instruction line/label if the zero flag is **not** set.  
*! warning - jumping to a number will not jump to the instruction, need to fix.*

#### JZ reg/imm
&rarr; `0x16 JZR` (jump to value in register)  
&rarr; `0x17 JZD` (jump to direct)  
Jumps to a specific instruction line/label if the zero flag is set.  
*! warning - jumping to a number will not jump to the instruction, need to fix.*

#### LD reg, mem/reg/imm
&rarr; `0x01 LDM` (load memory into register)  
&rarr; `0x02 LDR` (load register into register)  
&rarr; `0x03 LDD` (load value into register)  
Loads a value into a specified register.

#### NOP
&rarr; `0x00 NOP` (no operation)
Does nothing for 1 instruction cycle.

#### OR reg, reg/imm
&rarr; `0x0C ORR` (or register)  
&rarr; `0x0D ORD` (or direct)  
Performs the logical or | operator on two values.  

#### POP reg
&rarr; `0x1E POP` (pop register)   
Pops value at the top of the stack to a register, lowers the stack (stack_ptr--), leaves value behind.  

#### PSH reg/imm
&rarr; `0x1C PSHR` (push register)  
&rarr; `0x1D PSHD` (push direct)  
Pushes a value onto the stack, raises the stack (stack_ptr++).

#### STR mem, reg/imm
&rarr; `0x04 STR` (store register)  
&rarr; `0x05 STD` (store direct)  
Stores a value into memory. Can't store directly into variable space [0x00] -> [0x10].

#### SUB reg, reg/imm
&rarr; `0x08 SBR` (subtract register)  
&rarr; `0x09 SBD` (subtract direct)  
Subtracts src from dest and stores the result in dest.

#### XOR reg, reg/imm
&rarr; `0x0E XOR` (xor register)  
&rarr; `0x0F XOD` (xor direct)  
Performs the logical xor ^ operator on two values.
