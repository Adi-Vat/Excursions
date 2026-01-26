import instruction_set ::*;
// takes in the instruction
// determines which source the operand data should be taken from
// if the source is a register file, set the index of the required register
module control_unit(
	input bit clk,
	input bit reset,
	input logic [39:0] instruction,
	input logic [7:0] flags,
	
	output logic [1:0] rf_read_addr_a,
	output logic [1:0] rf_read_addr_b,
	output logic [1:0] rf_write_addr,
	output bit rf_write_enable
	output logic [1:0] alu_src_a_sel,
	output logic [1:0] alu_src_b_sel
);

logic [7:0] opcode = instruction[39:32];
logic [15:0] operand_a = instruction[31:16];
logic [15:0] operand_b = instruction[15:0];

always_comb begin
	rf_read_addr_a = 2'b00;
	rf_read_addr_b = 2'b00;
	alu_src_a_sel = 2'b00;
	alu_src_b_sel = 2'b00;
	
	case (opcode)
		LDD: begin
			rf_read_addr_a = 
			if(operand_b >= 16'h00A4 && operand_b <= 16'h00A7) begin
				rf_read_addr_b = operand_b[1:0];
				alu_src_b_sel = 2'b01;
			end
		end
	endcase
end


