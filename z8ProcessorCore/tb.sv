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

		while(!dut.cu.halted) repeat(1) @(posedge clk);
		
		for(int i = 0; i < 4; i++) $display("R%0d = 0x%2h", i, dut.rf.registers[i]);
		
		
		for(int y = 0; y < 16; y++) begin
			for(int x = 0; x < 16; x++) begin
				if(address == dut.cu.stack_ptr) stack_ptr_identifier = ">";
				else stack_ptr_identifier = " ";
				$write("|%s%02h:%02h ", stack_ptr_identifier, address[7:0], dut.mem.data_mem[address]);
				address++;
			end
			$write("|\n");
		end
		$write("\n");
		$display("Overflow: %b", dut.cu.flags.overflow);
		$display("Carry: %b", dut.cu.flags.carry);
		$display("Negative: %b", dut.cu.flags.negative);
		$display("Zero: %b", dut.cu.flags.zero);
		$display("SP: 0x%02h", dut.cu.stack_ptr);
		$write("\n");
		$finish;
	end
endmodule