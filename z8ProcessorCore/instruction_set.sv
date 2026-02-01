package instruction_set;
	parameter DATA_MEM_SIZE = 256;

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
		HALT = 8'hfe // 0xFE Stops program
	} OPCODES_T;
	
	typedef enum logic [3:0]{
		ALU_NOP,
		ALU_ADD,
		ALU_SUB,
		ALU_INC,
		ALU_DEC,
		ALU_AND,
		ALU_OR,
		ALU_XOR,
		ALU_CMP
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