`timescale 1ns / 1ps

module SquareRoot_tb;
    reg [31:0] floating_point_in;
    reg clk;
    wire [31:0] result;

    // Instantiate the square root module
    SquareRoot uut (
        .floating_point_in(floating_point_in),
        .clk(clk),
        .result(result)
    );

    // Clock
    initial begin
        clk = 0;
        forever #5 clk = ~clk; 
    end

    // Test cases
    initial 
    begin    
        $dumpfile("Squareroot.vcd");
        $dumpvars();

        $display("Time\tclk\tInput\tOutput");
        
        floating_point_in = 32'h41100000; // 9.0
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        floating_point_in = 32'h40490fda; // 3.141592
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        floating_point_in = 32'h00000000; // 0.0
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        floating_point_in = 32'h7F800000; // +INF
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        floating_point_in = 32'hC0800000; // -4.0
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        floating_point_in = 32'h40400000; // 3.0
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        floating_point_in = 32'h41800000; // 16.0
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        floating_point_in = 32'h5d94c9c2; // pi^pi^pi = 1.3401642e+18
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        floating_point_in = 32'hFF800000; // -INF
        #10 $display("%0t\t%b\t%h\t%h\n", $time, clk, floating_point_in, result);

        $finish;
    end
endmodule