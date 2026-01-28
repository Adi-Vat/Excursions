package instruction_set;
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
		CPD, // Compare reg and value
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