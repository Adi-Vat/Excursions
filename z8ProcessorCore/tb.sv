import instruction_set::*;
module tb;
	logic clk;
	logic reset;
	int address;
	
	z8ProcessorCore dut(
		.clk(clk),
		.reset(reset)
	);
	
	initial begin
		clk = 0;
		address = 0;
		forever #5 clk = ~clk;
	end
	
	initial begin
		reset = 1;
		#20;
		reset = 0;
		
		/*
		dut.mem.prog_mem[0] = {STD, 16'h0001, 16'h0021}; // STD #0001, 0x21
		dut.mem.prog_mem[1] = {LDM, 16'h0000, 16'h0001}; // LDM R0, #0001
		dut.mem.prog_mem[2] = {LDR, 16'h0001, 16'h0000}; // LDR R1, R0
		dut.mem.prog_mem[3] = {ADD, 16'h0001, 16'h0010}; // ADD R1, 0x10
		dut.mem.prog_mem[4] = {STR, 16'h0000, 16'h0001}; // STR #0000, R1
		dut.mem.prog_mem[5] = {LDD, 16'h0002, 16'h2300}; // LDD R2, 0x2300
		dut.mem.prog_mem[6] = {ADR, 16'h0000, 16'h0002}; // ADR R0, R2
		dut.mem.prog_mem[7] = {HALT, 16'h0, 16'h0};
		*/
		
		dut.mem.prog_mem[0] = {LDD, 16'h0, 16'h0};
		dut.mem.prog_mem[1] = {LDD, 16'h1, 16'h8000};
		dut.mem.prog_mem[2] = {SBR, 16'h0, 16'h1};
		dut.mem.prog_mem[3] = {HALT, 16'h0, 16'h0};
		
		while(!dut.cu.halted) repeat(1) @(posedge clk);
		
		for(int i = 0; i < 4; i++) $display("R%0d = %4h", i, dut.rf.registers[i]);
		
		
		for(int y = 0; y < 16; y++) begin
			$write("| ");
			for(int x = 0; x < 16; x++) begin
				//$write("%2h:%4h ", address,dut.mem.data_mem[address]);
				$write("%02h:%04h | ", address[7:0], dut.mem.data_mem[address]);
				address++;
			end
			$write("\n");
		end
		$write("\n");
		$display("Overflow: %b", dut.cu.flags.overflow);
		$display("Carry: %b", dut.cu.flags.carry);
		$display("Negative: %b", dut.cu.flags.negative);
		$display("Zero: %b", dut.cu.flags.zero);
		$write("\n");
		$finish;
	end
endmodule