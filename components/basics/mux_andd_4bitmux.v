module MUX(a, b, s, o);
	input a, b, s;
	output o;
	wire not_s, a_and_not_s, b_and_s; 

	not(not_s, s);
	and(a_and_not_s, a, not_s);
	and(b_and_s, b, s);
	or(o, b_and_s, a_and_not_s);
endmodule


module MUX4bit(a, b, c, d, s1, s2, o);
	input a, b, c, d, s1, s2;
	output o;
	wire not_s1, not_s2, a_out, b_out, c_out, d_out;

	not(not_s1, s1);
	not(not_s2, s2);

	and(a_out, a, not_s1, not_s2);
	and(b_out, b, not_s1, s2);
	and(c_out, c, s1, not_s2);
	and(d_out, d, s1, s2);

	or(o, a_out, b_out, c_out, d_out);
endmodule


module top;
	reg a, b, c, d, s1, s2;
	wire o;

  MUX4bit mux_inst(.a(a), .b(b), .c(c), .d(d), .s1(s1), .s2(s2), .o(o));

	initial begin
		$monitor("Time=%0t: a=%b, b=%b, c=%b, d=%b, s1=%b, s2=%b, o=%b", 
			$time, a, b, c, d, s1, s2, o);

		// Test all combinations
		repeat(64) begin
			{a, b, c, d, s1, s2} = $random;
			#10;
			check_output();
		end

		// Additional specific test cases
		a=1; b=0; c=0; d=0; s1=0; s2=0; #10; check_output();
		a=0; b=1; c=0; d=0; s1=0; s2=1; #10; check_output();
		a=0; b=0; c=1; d=0; s1=1; s2=0; #10; check_output();
		a=0; b=0; c=0; d=1; s1=1; s2=1; #10; check_output();

		$finish;
	end

	task check_output;
		reg expected;
		begin
			case({s1, s2})
				2'b00: expected = a;
				2'b01: expected = b;
				2'b10: expected = c;
				2'b11: expected = d;
			endcase

			if (o !== expected)
				$display("Error at time %0t: Expected %b, got %b", $time, expected, o);
			else
				$display("Correct output at time %0t", $time);
		end
	endtask
endmodule
