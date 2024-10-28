module XOR3In(a, b, c, o);
	input a, b, c;
	output o;
	wire not_a, not_b, not_c;
	wire and_a, and_b, and_c; // letter in the and is the one that's not negated

	not(not_a, a);
	not(not_b, b);
	not(not_c, c);

	and(and_a, a, not_b, not_c);
	and(and_b, not_a, b, not_c);
	and(and_c, not_a, not_b, c);

	or(and_a, and_b, and_c);
endmodule


// TODO: implement this bitch wtf
module Adder1Bit(a, b, o);
	input a, b;
	output o;
	wire sum, c_in, c_out, ca_out, cb_out;

	and(c_in, a, b);
	and(ca_out, a, c_in);
	and(cb_out, b, c_in);

	XOR3In xor_inst(sum, a, b, c_in);
	or(c_out, ca_out, cb_out);
endmodule
