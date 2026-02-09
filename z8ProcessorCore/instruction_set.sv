package instruction_set;
	parameter DATA_MEM_SIZE = 256;
	parameter PROG_MEM_SIZE = 256;
	parameter WORD_SIZE = 8;
	parameter INSTRUCTION_SIZE = 40;
	parameter OPCODE_MSB = 39;
	parameter OPCODE_LSB = 32;
	parameter OPERAND_A_MSB = 31;
	parameter OPERAND_A_LSB = 16;
	parameter OPERAND_B_MSB = 15;
	parameter OPERAND_B_LSB = 0;
	
	parameter STACK_START = 8'hFF;
	parameter STACK_END = 8'hF0;
	parameter BANK_0_START = 8'hD9;
	parameter BANK_0_END = BANK_0_START + 8'h02;
	parameter BANK_1_START = BANK_0_END + 8'h01;
	parameter BANK_1_END = BANK_1_START + 8'h03;
	parameter CONTROL_FLAGS_ADDR = 8'h00;
	
	typedef enum logic [7:0]{
		BANK_SEL = 8'b0
	} CF_BITS;
	
	typedef enum logic[6:0]{
		DHEX0 = ~(7'b0111111),
		DHEX1 = ~(7'b0000110),
		DHEX2 = ~(7'b1011011),
		DHEX3 = ~(7'b1001111),
		DHEX4 = ~(7'b1100110),
		DHEX5 = ~(7'b1101101),
		DHEX6 = ~(7'b1111101),
		DHEX7 = ~(7'b0000111),
		DHEX8 = ~(7'b1111111),
		DHEX9 = ~(7'b1100111),
		DHEXA = ~(7'b1110111),
		DHEXB = ~(7'b1111100),
		DHEXC = ~(7'b0111001),
		DHEXD = ~(7'b1011110),
		DHEXE = ~(7'b1111001),
		DHEXF = ~(7'b1110001)
	} HEX_DIGITS;

	typedef enum logic[7:0]
	{
		NOP, // 0x00 NO OPERATION
		LDM, // 0x01 Load value from memory into register
		LDR, // 0x02 Load value from register into register
		LDD, // 0x03 Load value directly into register
		STR, // 0x04 Store value from register into memory address
		STD, // 0x05 Store direct value into memory address
		ADR, // 0x06 Add value from register to another register
		ADD, // 0x07 Add value to another register
		SBR, // 0x08 Subtract the source from the dest and store in the dest
		SBD, // 0x09 Subtract a value from the register and store in the register
		ANR, // 0x0A AND a register and a register store in the dest
		AND, // 0x0B AND a register and value store in the dest
		ORR, // 0x0C OR a reg and reg store in dest
		ORD, // 0x0D OR a reg and value store in dest
		XOR, // 0x0E XOR a reg and reg store in dest
		XOD, // 0x0F XOR a reg and value store in dest
		CPR, // 0x10 Compare reg and reg
		CPD, // 0x11 Compare reg and value,
		JPR, // 0x12 Jump to line stored in reg
		JPD, // 0x13 Jump directly to line
		JZR, // 0x14 Jump to line stored in reg if zero flag is set
		JZD, // 0x15 Jump directly to line if zero flag is set
		JNZR, // 0x16 Jump to line stored in reg if zero flag is NOT set
		JNZD, // 0x17 Jump directly to line if zero flag is NOT set
		JNR, // 0x18 Jump to line stored in reg if negative flag is set
		JND, // 0x19 Jump directly to line if negative flag is set
		INC, // 0x1A Increment register
		DEC, // 0x1B Decrement register
		PSHR, // 0x1C Push register to stack
		PSHD, // 0x1D Push value to stack
		POP, // 0x1E Pop top value from stack into register
		SBIM, // 0x1F Set bit of word in memory (to 1)
		SBIR, // 0x20 Set bit of word in register (to 1)
		CBIM, // 0x21 Clear bit of word in memory (to 0)
		CBIR, // 0x22 Clear bit of word in register (to 0)
		HALT = 8'hfe // 0xFE Stops program
	} OPCODES_T;
	
	typedef enum logic [3:0]{
		ALU_NOP, // 0 
		ALU_ADD, // 1
		ALU_SUB, // 2
		ALU_INC, // 3
		ALU_DEC, // 4
		ALU_AND, // 5
		ALU_OR, // 6
		ALU_XOR, // 7
		ALU_CMP, // 8
		ALU_SB, // 9
		ALU_CB // 10
	} ALU_OPS_T;
	
	typedef enum logic[1:0]{
		VAL,
		REG,
		MEM
	} DATA_SOURCE_T;
	
	typedef enum logic[3:0]{
		NONE,
		ALU
	} FLAG_SOURCE_T;
	
	typedef enum logic [1:0]{
		MEM_NOP,
		MEM_READ,
		MEM_WRITE
	} MEM_OPS_T;
	
	typedef enum logic [1:0]{
		FETCH,
		DECODE,
		EXECUTE,
		WRITEBACK
	} STATE_T;
	
	typedef struct packed{
		logic overflow; // 3
		logic carry; // 2
		logic negative; // 1
		logic zero; // 0
	} FLAGS_T;
endpackage