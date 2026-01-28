import instruction_set ::*;

module arithmetic_logic_unit(
	input logic [15:0] in_a,
	input logic [15:0] in_b,
	input ALU_OPS_T op,
	output logic [15:0] out,
	output FLAGS_T flags_out
);

logic [16:0] temp_out;
logic carry_bit;

always_comb begin
	flags_out = FLAGS_T'(0);
	out = 16'b0;
	temp_out = 17'b0;
	case(op)
		ALU_ADD: temp_out = in_a + in_b;
		ALU_SUB, ALU_CMP: temp_out = in_a - in_b;
		ALU_AND: temp_out = in_a & in_b;
		ALU_OR: temp_out = in_a | in_b;
		ALU_XOR: temp_out = in_a ^ in_b;
	endcase
	
	
	out = temp_out[15:0];
	carry_bit = temp_out[16];
	
	
	case (op)
		ALU_ADD: begin
			if((in_a[15] == in_b[15]) && (in_a[15] != out[15])) flags_out.overflow = 1;
			if(carry_bit) flags_out.carry = 1;
		end
		ALU_SUB, ALU_CMP: begin
			if(in_a[15] == in_b[15]) begin
				if(in_a[15] == 0 && in_a < in_b) flags_out.carry = 1;
				if(in_a[15] == 1 && in_b < in_a) flags_out.carry = 1;
			end
			else begin 
				if(in_a[15] == 1 && in_b[15] == 0) flags_out.carry = 1;
				if(in_a[15] != out[15])flags_out.overflow = 1;
			end
		end
	endcase
	
	if(out == 0) flags_out.zero = 1;
	if(out[15]) flags_out.negative = 1;
	
end

endmodule
