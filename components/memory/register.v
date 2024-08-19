// the clock should be built-in, or something
module smol_reg(load, in, out);
	input load, in;
	output out;

	wire mux_out;
	// out = in * load + out * load' --> load = 1, select in, load = 0, continue using the same output from last clock tick
	// load = 0; t+1 = t, load = 1; t+1 = in != t

	Mux(mux_out, in, out, load);
	DFF(out, mux_out);
endmodule
