import instruction_set ::*;

module z8ProcessorCore(
	input logic clk,
	input logic reset
	// Add external I/O for FPGA here
);

	logic [39:0] instruction;
	logic [39:0] instr_reg;
	logic [15:0] pc;
	logic [15:0] next_pc;
	STATE_T current_state;
	FLAGS_T current_flags;
	FLAG_SOURCE_T flag_read_src_sel;
	bit update_flags;

	bit rf_write_enable;
	logic [1:0] rf_read_addr_a;
	logic [1:0] rf_read_addr_b;
	logic [1:0] rf_write_addr;
	logic [15:0] rf_read_data_a;
	logic [15:0] rf_read_data_b;
	logic [15:0] rf_write_data;
	
	MEM_OPS_T mem_op;
	logic [15:0] mem_addr;
	logic [15:0] mem_read_data;
	logic [15:0] mem_write_data;
	
	ALU_OPS_T alu_op;
	logic [15:0] alu_in_data_a;
	logic [15:0] alu_in_data_b;
	logic [15:0] alu_out;
	DATA_SOURCE_T alu_a_src_sel;
	DATA_SOURCE_T alu_b_src_sel;
	
	FLAGS_T alu_flags;
	
	assign instr_reg = instruction;
	
	always_ff @(posedge clk) begin
		pc <= next_pc;
	end

	
	// FLAG MUX
	// Decides which module should set the flag in this instruction cycle
	always_comb begin
		case(flag_read_src_sel)
			ALU:
				current_flags = alu_flags;
		endcase
	end
	
	control_unit cu(
		.instruction(instr_reg),
		.clk(clk),
		.reset(reset),
		.pc(pc),
		.flags_in(current_flags),
		.update_flags(update_flags),
		.rf_read_data_a(rf_read_data_a),
		
		.next_pc(next_pc),
		.current_state(current_state),
		.rf_read_addr_a(rf_read_addr_a),
		.rf_read_addr_b(rf_read_addr_b),
		.rf_write_addr(rf_write_addr),
		.rf_write_enable(rf_write_enable),
		.mem_rw_addr(mem_addr),
		.mem_op(mem_op),
		.alu_op(alu_op),
		.alu_a_src_sel(alu_a_src_sel),
		.alu_b_src_sel(alu_b_src_sel)
	);
	
	
	memory_manager mem(
		.clk(clk),
		.reset(reset),
		.pc(pc),
		.op(mem_op),
		.addr(mem_addr),
		.write_data(mem_write_data),
		.read_data(mem_read_data),
		.current_instruction(instruction)
	);
	
	
	// Writeback MUX
	// each instruction writes something back somewhere
	always_comb begin
		rf_write_data = 0;
		mem_write_data = 0;
		update_flags = 0;
		flag_read_src_sel = NONE;
		
		case(instr_reg[39:32])
			// write data from memory
			// data from memory already gotten by CU
			LDM, POP: rf_write_data = mem_read_data;
			// write data from register
			// location of data from register is exposed by the CU
			// and the data is found by the RF, and exposed
			LDR: rf_write_data = rf_read_data_b;
			// write absolute value from instruction
			LDD: rf_write_data = instr_reg[15:0];
			// write memory location from RF out b
			STR: mem_write_data = rf_read_data_b;
			// write memory location with value from instruction
			STD: mem_write_data = instr_reg[15:0];
			// write data from output of ALU
			ADR, ADD, SBR, SBD, ANR, AND, ORR, ORD, XOR, XOD, CPR, CPD,
			INC, DEC: begin
				rf_write_data = alu_out;
				flag_read_src_sel = ALU;
				update_flags = 1;
			end
			// write the value of the destination register to memory
			// the dest portion as PSHR only has 1 argument
			PSHR: mem_write_data = rf_read_data_a;
			// write the dest value to memory, as PSHD only has 1 argument
			PSHD: mem_write_data = instr_reg[31:16];
		endcase
	end
	
	register_file rf(
		.clk(clk),
		.reset(reset),
		.read_addr_a(rf_read_addr_a),
		.read_addr_b(rf_read_addr_b),
		.write_addr(rf_write_addr),
		.write_data(rf_write_data),
		.write_enable(rf_write_enable),
		.read_data_a(rf_read_data_a),
		.read_data_b(rf_read_data_b)
	);
	
	// ALU MUX
	always_comb begin
		alu_in_data_a = 0;
		alu_in_data_b = 0;
		// correctly get data for alu in a
		case (alu_a_src_sel)
			REG: alu_in_data_a = rf_read_data_a;
		endcase
		// for alu in b
		case (alu_b_src_sel)
			REG: alu_in_data_b = rf_read_data_b;
			VAL: alu_in_data_b = instr_reg[15:0];
			MEM: alu_in_data_b = mem_read_data;
		endcase
	end
	
	arithmetic_logic_unit alu(
		.in_a(alu_in_data_a),
		.in_b(alu_in_data_b),
		.op(alu_op),
		.out(alu_out),
		.flags_out(alu_flags)
	);
endmodule
	