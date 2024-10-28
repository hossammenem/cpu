module sign_extend_8_to_16 (
    input [7:0] in,
    output [15:0] out
);
  assign out[7:0] = in[7:0];

	wire msb;
	assign msb = in[7];

	// in physical world, this is like having 9 wires connected to the msb wire, one is in it's normal place, and the others are for sign extension
	assign out[15:8] = {8{in[7]}}; 
endmodule
