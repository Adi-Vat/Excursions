import instruction_set ::*;

module memory_manager(
	input bit clk,
	input bit reset,
	input logic [15:0] pc,
	input logic [1:0] op,
	input logic [15:0] addr,
	input logic [15:0] write_data,
	output logic [15:0] read_data,
	output logic [39:0] current_instruction
);

logic [15:0] data_mem [256];
logic [39:0] prog_mem [256];

always_ff @(posedge clk) begin
	case (op)
		MEM_WRITE: data_mem[addr] <= write_data;
	endcase
	
	if(reset) begin
		for(int i = 0; i < 256; i++) data_mem[i] <= 0;
	end
end

always_comb begin
	read_data = 0;
	
	case(op)
		MEM_READ: read_data = data_mem[addr];
	endcase
	
	current_instruction = prog_mem[pc];
end

endmodule