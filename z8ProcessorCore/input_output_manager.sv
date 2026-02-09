import instruction_set ::*;

module input_output_manager(
	input logic [2:0][WORD_SIZE-1:0] bank_0_in,
	input logic [3:0][WORD_SIZE-1:0] bank_1_in,
	input bit bank_sel,
	output logic [5:0][6:0] hex_out
);

logic [1:0][11:0] bank_1_temp;
int newI;

always_comb begin
	for (int i = 0; i < 5; i++) hex_out[i] = 0;
	for (int i = 0; i < 2; i++) bank_1_temp = 0;
	// direct value to hex out
	if(bank_sel == 0) begin
		for (int i = 0; i < 3; i++) begin
			hex_out[i*2] = HEX_DIGITS'(bank_0_in[i][3:0]);
			hex_out[i*2 + 1] = HEX_DIGITS'(bank_0_in[i][7:4]);
		end
	end
	else begin
		for (int x = 0; x < 6; x++) begin
			for(int y = 0; y < 4; y++) begin
				bank_1_temp[y>>1][x + (6*y%2)] = bank_1_in[y][x];
			end
		end
		
		for (int i = 0; i < 6; i++) begin
			hex_out[i][1] = bank_1_temp[0][i*2];
			hex_out[i][2] = bank_1_temp[1][i*2];
			hex_out[i][5] = bank_1_temp[0][i*2 + 1];
			hex_out[i][4] = bank_1_temp[1][i*2 + 1];
		end
	end
end


endmodule