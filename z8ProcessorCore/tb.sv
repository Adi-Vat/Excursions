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
		
		
		dut.mem.prog_mem[0] = {ADD, 16'h0000, 16'h0001}; // ADD R0, 1
		dut.mem.prog_mem[1] = {CPD, 16'h0000, 16'h0001}; // CMP R0, 1
		dut.mem.prog_mem[2] = {JNZD, 16'h0004, 16'h0000}; // JNZ 4
		dut.mem.prog_mem[3] = {ADD, 16'h0000, 16'h0001}; // ADD R0, 1
		dut.mem.prog_mem[4] = {HALT, 16'h0000, 16'h0000}; // HALT
		
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