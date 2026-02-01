# Z8 Assembler (v2)
## Overview
An assembler for my own assembly language, z8.  
Converts an assembly file to a hex file.
The hex file can then be run by my [z8 Processor Core](/z8ProcessorCore/)  

## Syntax and Features
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
To run the assembler use 
