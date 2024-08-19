module MUX3Sel(a, b, s1, s2, s3, o);

endmodule

// zx, nx, zy, ny, zr, and ng
module ALU(a, b, op, o);
	input a, b, [2:0] op;
	output o;
	wire adder, subber, incer, decer, notter, andder, orer, xorer;


	// op[0], op[1], op[2] // this is s1, s2, s3
	// Adder();
	// Adder(); // but sub here
	// inc
	// dec
	// not
	// and
	// or
	// xor 
	MUX3Sel f_sel(o, a, b, op[0], op[1], op[2]);

endmodule
