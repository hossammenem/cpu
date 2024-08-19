module sr_latch(
    input S, R,
    output Q, Qbar
);
		wire not_s, not_r;
    nor(Qbar, R, Q);
		not(not_s, S);
		not(not_r, R);

    nand(Q, not_s, Qbar); 
		nand(Qbar, not_r, Q);
endmodule
