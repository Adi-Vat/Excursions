z8_processor(
	input logic clk,
	input logic reset,
	// Add external I/O for FPGA here
);

	logic [39:0] instruction;
	logic [15:0] pc;
	