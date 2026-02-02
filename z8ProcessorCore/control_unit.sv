import instruction_set ::*;

module control_unit(
	input logic [39:0] instruction,
	input logic clk,
	input bit reset,
	input logic [WORD_SIZE-1:0] pc,
	input FLAGS_T flags_in,
	input bit update_flags,
	input logic [WORD_SIZE-1:0] rf_read_data_a,
	
	output logic [WORD_SIZE-1:0] next_pc,
	output STATE_T current_state,
	output bit rf_write_enable,
	output logic [1:0] rf_write_addr,
	output logic [1:0] rf_read_addr_a,
	output logic [1:0] rf_read_addr_b,
		
	output logic [WORD_SIZE-1:0] mem_rw_addr,
	output MEM_OPS_T mem_op,

	output ALU_OPS_T alu_op,
	output DATA_SOURCE_T alu_a_src_sel,
	output DATA_SOURCE_T alu_b_src_sel
);

OPCODES_T opcode;
assign opcode = OPCODES_T'(instruction[39:32]);
logic [WORD_SIZE-1:0] dest;
assign dest = instruction[31:16];
logic [WORD_SIZE-1:0] src;
assign src = instruction[WORD_SIZE-1:0];
bit halted;

FLAGS_T flags;
STATE_T next_state;

logic [WORD_SIZE-1:0] stack_ptr;

always_ff @(posedge clk) begin
	if(reset) begin
		current_state <= FETCH;
		stack_ptr <= DATA_MEM_SIZE - 1;
	end else if(!halted) begin
		current_state <= next_state;
	end
	
	if(update_flags) begin
		flags <= flags_in;
	end
	
	case (current_state)
		FETCH: if(opcode == HALT) halted <= 1;
		DECODE: begin
			case (opcode)
				// Load data from memory into register
				// set reading address for memory
				LDM: mem_rw_addr <= src;
				// Load data from register into register
				// set reading address from register
				LDR: rf_read_addr_b <= src;
				// Store data from register into memory
				// set reading address from register
				STR: begin
					rf_read_addr_b <= src;
					mem_rw_addr <= dest;
				end
				// Store value directly into memory
				STD: mem_rw_addr <= dest;
				// Add two registers and save the output in the dest
				ADR, SBR, ANR, ORR, XOR, CPR: begin
					rf_read_addr_a <= dest;
					rf_read_addr_b <= src;
				end
				// Add a value to a register and store in that register
				ADD, SBD, AND, ORD, XOD, CPD, JPR, JZR, JNZR, JNR, INC, DEC: begin
					rf_read_addr_a <= dest;
				end
				// Get value from register, write value to memory, dec stack pointer
				PSHR: begin
					rf_read_addr_a <= dest;
					mem_rw_addr <= stack_ptr;
					if(stack_ptr > 0) stack_ptr <= stack_ptr - 1;
					
				end
				// Dec stack pointer and set memory write address to the pointer
				PSHD: begin
					mem_rw_addr <= stack_ptr;
					if(stack_ptr > 0) stack_ptr <= stack_ptr - 1;
				end
				// Inc stack pointer, set memory read address to pointer, write back read value to register
				POP: begin
					if(stack_ptr < DATA_MEM_SIZE - 1) begin
						stack_ptr <= stack_ptr + 1;
						mem_rw_addr <= stack_ptr + 1;
					end else begin
						mem_rw_addr <= stack_ptr;
					end
				end
			endcase
		end
		
		EXECUTE: begin
			case (opcode)
				// tell memory to read
				LDM, POP: mem_op <= MEM_READ;
				// tell memory to write
				STR, STD, PSHR, PSHD: mem_op <= MEM_WRITE;
				ADR: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= REG;
					alu_op <= ALU_ADD;
				end
				ADD: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= VAL;
					alu_op <= ALU_ADD;
				end
				SBR: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= REG;
					alu_op <= ALU_SUB;
				end
				SBD: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= VAL;
					alu_op <= ALU_SUB;
				end
				ANR: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= REG;
					alu_op <= ALU_AND;
				end
				AND: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= VAL;
					alu_op <= ALU_AND;
				end
				ORR: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= REG;
					alu_op <= ALU_OR;
				end
				ORD: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= VAL;
					alu_op <= ALU_OR;
				end
				XOR: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= REG;
					alu_op <= ALU_XOR;
				end
				XOD: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= VAL;
					alu_op <= ALU_XOR;
				end
				CPR: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= REG;
					alu_op <= ALU_CMP;
				end
				CPD: begin
					alu_a_src_sel <= REG;
					alu_b_src_sel <= VAL;
					alu_op <= ALU_CMP;
				end
				INC: begin
					alu_a_src_sel <= REG;
					alu_op <= ALU_INC;
				end
				DEC: begin
					alu_a_src_sel <= REG;
					alu_op <= ALU_DEC;
				end
			endcase
		end
		
		WRITEBACK: begin
			rf_read_addr_a <= 0;
			rf_read_addr_b <= 0;
			mem_rw_addr <= 0;
			mem_op <= MEM_NOP;
			alu_op <= ALU_NOP;
			alu_a_src_sel <= DATA_SOURCE_T'(0);
			alu_b_src_sel <= DATA_SOURCE_T'(0);
			alu_b_src_sel <= DATA_SOURCE_T'(0);
		end
		
	endcase
end

always_comb begin
	rf_write_addr = 0;
	rf_write_enable = 0;
	if(reset) next_pc = 0;
	else if(current_state == WRITEBACK) next_pc = pc + 1;
	else next_pc = pc;

	case (current_state)
		FETCH: next_state = DECODE;
		DECODE: next_state = EXECUTE;
		EXECUTE: next_state = WRITEBACK;
		WRITEBACK: begin
			case(opcode)
				ADD, ADR, LDM, LDR, LDD, SBR, SBD, ANR, AND, ORR, ORD,
				XOR, XOD, INC, DEC, POP: begin
					// write memory read result to correct location
					rf_write_addr = dest;
					// enable memory writing
					rf_write_enable = 1;
				end
				JPR: next_pc = rf_read_data_a;
				JPD: next_pc = dest;
				JZR: if(flags.zero) next_pc = rf_read_data_a;
				JZD: if(flags.zero) next_pc = dest;
				JNZR: if(!flags.zero) next_pc = rf_read_data_a;
				JNZD: if(!flags.zero) next_pc = dest;
				JNR: if(flags.negative) next_pc = rf_read_data_a;
				JND: if(flags.negative) next_pc = dest;
			endcase
			next_state = FETCH;
		end
	endcase
	
	/*
		LDM:
		Load value from memory into register
		decode location in memory
		Execute read operation
					
		LDR: 
		Load value from register into register
		need the address of the register to read from
		need the address of the register to write to
		rf write enable
	
		LDD:
		Load value directly into register
		need the address of the register to write to
		rf write enable
		
		STR:
		Store value from register into memory
		need the value at the register address
		tell memory module to write stuff
		
		STD:
		Store value directly into memory
		tell memory module to write
		
		ADR, SBR:
		Add (or subtract) the number in a register to another register
		Use the RF to get the value at the src and dest
		write the output of this operation to the dest
		enable rf writing
		dest <- [dest] +(-) [source]
		read
		alu_a = rf_addr(dest)
		alu_b = rf_addr(source)
		compute
		alu_out = alu_a +(-) alu_b
		writeback
		rf_addr(dest) = alu_out
		
		ADD, SBD:
		Add (or sub) the a value to a register
		Need to specify the address of the destination register
		tell the ALU MUX that value A data source is a register
		tell the ALU MUX that value B data source is a direct value

	*/
end

endmodule 
