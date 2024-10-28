module DFF(C, D);
	wire not_c, w_latch1, w_latch2;

	not(not_c, C);
	D_latch latch1(w_latch1, D, C);
	D_latch latch2(Q, w_latch1, not_c);
endmodule
