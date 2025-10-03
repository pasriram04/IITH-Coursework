module fixedPointSquareroot_tb;
    reg [22:0] Y;
    reg clk;
    wire [23:0] result;

    fixedPointSquareroot uut (
        .Y(Y),
        .clk(clk),
        .result(result)
    );

    initial begin
        clk = 1;
        forever #5 clk = ~clk;
    end

    initial begin
        $dumpfile("fixedPointSquareroot.vcd");
        $dumpvars();
        
        $display("Time\tclk\tY\tResult");
        
        #10 Y = 23'h400000; // Example input (e.g., 1.5 in fixed point)
        #10 Y = 23'h600000; // Example input (e.g., 1.75 in fixed point)
        #10 Y = 23'h7FF000; // Maximum value for testing
        #10 Y = 23'h001000; // Minimum value
        #10 Y = 23'h000000; // Minimum value
        #10 Y = 23'h3FFFFF; // Mid-range value
        #10 Y = 23'h5AAAAA; // Another mid-range value
        #5;
        $finish;
    end
endmodule