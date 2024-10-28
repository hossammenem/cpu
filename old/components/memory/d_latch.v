module D_latch(D, C);
	input C, D;
	output Q, Qbar;

	wire not_d, upper_and, lower_and;

	not(not_d, D);

	and(upper_and, not_d, C);
	and(lower_and, D, C);

	nor(Q, upper_and, Qbar);
	nor(Qbar, lower_and, Q);
endmodule
