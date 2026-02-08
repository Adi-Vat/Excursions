import instruction_set ::*;

module memory_manager(
	input bit clk,
	input bit reset,
	input logic [WORD_SIZE-1:0] pc,
	input logic [1:0] op,
	input logic [WORD_SIZE-1:0] addr,
	input logic [WORD_SIZE-1:0] write_data,
	output logic [WORD_SIZE-1:0] read_data,
	output logic [INSTRUCTION_SIZE-1:0] current_instruction,
	output logic [WORD_SIZE-1:0] bank_0,
	output logic [WORD_SIZE-1:0] bank_1,
	output bit bank_sel
);

logic [DATA_MEM_SIZE-1:0][WORD_SIZE-1:0] data_mem;
logic [INSTRUCTION_SIZE-1:0] prog_mem [0:255];

always_ff @(posedge clk) begin
	case (op)
		MEM_WRITE: data_mem[addr] <= write_data;
	endcase
	
	if (reset) begin
		$readmemh("C:/Users/Adi/Documents/Excursions/z8ProcessorCore/programs/fibonacci.hex", prog_mem);	
		for(int i = 0; i < 256; i++) data_mem[i] <= 0;
	end
end

always_comb begin
	read_data = 0;
	
	case(op)
		MEM_READ: read_data = data_mem[addr];
	endcase
	
	current_instruction = prog_mem[pc];
	bank_0 = data_mem[BANK_0_END:BANK_0_START];
	bank_1 = data_mem[BANK_1_END:BANK_1_START];
	bank_sel = data_mem[CONTROL_FLAGS_ADDR][CF_BITS'(BANK_SEL)];
end

endmodule