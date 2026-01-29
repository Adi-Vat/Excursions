package instruction_set;
	parameter DATA_MEM_SIZE = 256;

	typedef enum logic[7:0]
	{
		NOP, // NO OPERATION
		LDM, // Load value from memory into register
		LDR, // Load value from register into register
		LDD, // Load value directly into register
		STR, // Store value from register into memory address
		STD, // Store direct value into memory address
		ADR, // Add value from register to another register
		ADD, // Add value to another register
		SBR, // Subtract the source from the dest and store in the dest
		SBD, // Subtract a value from the register and store in the register
		ANR, // AND a register and a register store in the dest
		AND, // AND a register and value store in the dest
		ORR, // OR a reg and reg store in dest
		ORD, // OR a reg and value store in dest
		XOR, // XOR a reg and reg store in dest
		XOD, // XOR a reg and value store in dest
		CPR, // Compare reg and reg
		CPD, // Compare reg and value,
		JPR, // Jump to line stored in reg
		JPD, // Jump directly to line
		JZR, // Jump to line stored in reg if zero flag is set
		JZD, // Jump directly to line if zero flag is set
		JNZR, // Jump to line stored in reg if zero flag is NOT set
		JNZD, // Jump directly to line if zero flag is NOT set
		JNR, // Jump to line stored in reg if negative flag is set
		JND, // Jump directly to line if negative flag is set
		INC, // Increment register
		DEC, // Decrement register
		PSHR, // Push register to stack
		PSHD, // Push value to stack
		POP, // Pop top value from stack into register
		HALT // Stops program
	} OPCODES_T;
	
	typedef enum logic [3:0]{
		ALU_NOP,
		ALU_ADD,
		ALU_SUB,
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