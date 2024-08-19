module sr_latch_tb;
    reg S, R;
    wire Q, Qbar;

    // Instantiate the latch
    sr_latch uut (
        .S(S),
        .R(R),
        .Q(Q),
        .Qbar(Qbar)
    );

    // Test sequence
    initial begin
        $display("Time\tS\tR\tQ\tQbar");
        $display("----\t-\t-\t-\t----");

        // Initialize the latch (Reset)
        S = 0; R = 1;
        #1; display_outputs;

        // Set to known state
        S = 1; R = 0;
        #1; display_outputs;

        // Hold
        S = 0; R = 0;
        #1; display_outputs;

        // Reset
        S = 0; R = 1;
        #1; display_outputs;

        // Hold
        S = 0; R = 0;
        #1; display_outputs;

        // Set
        S = 1; R = 0;
        #1; display_outputs;

        // Hold
        S = 0; R = 0;
        #1; display_outputs;

        // Invalid state (both S and R high)
        S = 1; R = 1;
        #1; display_outputs;

        $finish;
    end

    // Task to display outputs
    task display_outputs;
        $display("%0t\t%b\t%b\t%b\t%b", $time, S, R, Q, Qbar);
    endtask

endmodule
