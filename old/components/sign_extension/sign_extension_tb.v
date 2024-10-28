module sign_extend_tb;
    reg [7:0] in;
    wire [15:0] out;

    sign_extend_8_to_16 uut (
        .in(in),
        .out(out)
    );

    initial begin
        // Test positive number
        in = 8'b01010101;
        #10;
        $display("Input: %b, Output: %b", in, out);

        // Test negative number
        in = 8'b10101010;
        #10;
        $display("Input: %b, Output: %b", in, out);
    end
endmodule
