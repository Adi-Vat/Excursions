import instruction_set ::*;

module arithmetic_logic_unit(
	input logic [WORD_SIZE-1:0] in_a,
	input logic [WORD_SIZE-1:0] in_b,
	input ALU_OPS_T op,
	output logic [WORD_SIZE-1:0] out,
	output FLAGS_T flags_out
);

logic [WORD_SIZE:0] temp_out;
logic carry_bit;
bit in_a_msb;
bit in_b_msb;
bit out_msb;

always_comb begin
	flags_out = FLAGS_T'(0);
	out = 16'b0;
	temp_out = 17'b0;
	case(op)
		ALU_ADD: temp_out = in_a + in_b;
		ALU_SUB, ALU_CMP: temp_out = in_a - in_b;
		ALU_INC: temp_out = in_a + 1;
		ALU_DEC: temp_out = in_a - 1;
		ALU_AND: temp_out = in_a & in_b;
		ALU_OR: temp_out = in_a | in_b;
		ALU_XOR: temp_out = in_a ^ in_b;
	endcase
	
	
	out = temp_out[WORD_SIZE-1:0];
	carry_bit = temp_out[WORD_SIZE];
	
	
	in_a_msb = in_a[WORD_SIZE-1];
	in_b_msb = in_b[WORD_SIZE-1];
	out_msb = out[WORD_SIZE-1];
	
	case (op)
		ALU_ADD: begin
			if((in_a_msb == in_b_msb) && (in_a_msb != out_msb)) flags_out.overflow = 1;
			if(carry_bit) flags_out.carry = 1;
		end
		ALU_SUB, ALU_CMP: begin
			if(in_a_msb == in_b_msb) begin
				if(in_a_msb == 0 && in_a < in_b) flags_out.carry = 1;
				if(in_a_msb == 1 && in_b < in_a) flags_out.carry = 1;
			end
			else begin 
				if(in_a_msb == 1 && in_b_msb == 0) flags_out.carry = 1;
				if(in_a_msb != out_msb)flags_out.overflow = 1;
			end
		end
	endcase
	
	if(out == 0) flags_out.zero = 1;
	if(out_msb) flags_out.negative = 1;
	
end

endmodule
