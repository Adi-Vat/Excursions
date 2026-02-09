import instruction_set ::*;

module register_file(
	input logic clk,
	input logic reset,
	// rf address of data to read
	input logic [1:0] read_addr_a,
	input logic [1:0] read_addr_b,
	// rf address of data to write
	input logic [1:0] write_addr,
	// actual data to be written
	input logic [WORD_SIZE-1:0] write_data,
	input logic write_enable,
	// output of datas to be read
	output logic [WORD_SIZE-1:0] read_data_a,
	output logic [WORD_SIZE-1:0] read_data_b
);

logic [WORD_SIZE-1:0] registers [0:3];

always_ff @(posedge clk) begin
	if (reset) begin
		for(int i = 0; i < 4; i++)
			registers[i] <= 16'h0;
	end else if (write_enable) begin
		registers[write_addr] <= write_data;
	end
end

assign read_data_a = registers[read_addr_a];
assign read_data_b = registers[read_addr_b];

endmodule