import instruction_set::*;
module tb;
	logic clk;
	logic reset;
	int address;
	string stack_ptr_identifier;
	
	z8ProcessorCore dut(
		.clk(clk),
		.reset(reset)
	);
	
	initial begin
		clk = 0;
		address = 0;
		stack_ptr_identifier = " ";
		forever #5 clk = ~clk;
	end
	
	initial begin
		reset = 1;
		#20;
		reset = 0;
		
		dut.mem.prog_mem[0] = {LDD, 16'h0, 16'h01}; // LDD R0, 1
		dut.mem.prog_mem[1] = {PSHR, 16'h0, 16'h0}; // PUSH R0
		dut.mem.prog_mem[2] = {PSHD, 16'h2, 16'h0}; // PUSH 2
		dut.mem.prog_mem[3] = {POP, 16'h1, 16'h0}; // POP R1
		dut.mem.prog_mem[4] = {HALT, 16'h0000, 16'h0000}; // HALT
		
		while(!dut.cu.halted) repeat(1) @(posedge clk);
		
		for(int i = 0; i < 4; i++) $display("R%0d = %4h", i, dut.rf.registers[i]);
		
		
		for(int y = 0; y < 16; y++) begin
			for(int x = 0; x < 16; x++) begin
				//$write("%2h:%4h ", address,dut.mem.data_mem[address]);
				if(address == dut.cu.stack_ptr) stack_ptr_identifier = ">";
				else stack_ptr_identifier = " ";
				$write("|%s%02h:%04h ", stack_ptr_identifier, address[7:0], dut.mem.data_mem[address]);
				address++;
			end
			$write("|\n");
		end
		$write("\n");
		$display("Overflow: %b", dut.cu.flags.overflow);
		$display("Carry: %b", dut.cu.flags.carry);
		$display("Negative: %b", dut.cu.flags.negative);
		$display("Zero: %b", dut.cu.flags.zero);
		$display("SP: %04h", dut.cu.stack_ptr);
		$write("\n");
		$finish;
	end
endmodule