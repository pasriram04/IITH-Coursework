`timescale 1ns / 1ps

module SquareRoot(
    input [31:0] floating_point_in,
    input clk,
    output reg [31:0] result
);
    wire sign = floating_point_in[31];
    wire [7:0] exponent = floating_point_in[30:23];
    wire [22:0] mantissa = floating_point_in[22:0];

    wire is_nan = (exponent == 8'hFF) && (mantissa != 0);
    wire is_pos_inf = (sign == 1'b0) && (exponent == 8'hFF) && (mantissa == 0);
    wire is_neg_inf = (sign == 1'b1) && (exponent == 8'hFF) && (mantissa == 0);
    wire is_zero = (exponent == 8'h00) && (mantissa == 0);
    
    reg odd_flag = 1'b0;

    reg [7:0] sqrt_exponent;
    reg [23:0] sqrt_mantissa;
    reg [23:0] adjusted_mantissa;
    reg [46:0] product;
    
    wire [23:0] fixed_sqrt_result;
    fixedPointSquareroot sqrt_inst (
        .Y(adjusted_mantissa[22:0]),
        .clk(clk),
        .result(fixed_sqrt_result)
    );

    parameter ROOT2 = 24'hB504F3; // Fixed-point representation of sqrt(2) * 2^23

    always @* begin
        if (is_nan) begin
            result = 32'h7FC00000;
        end else if (is_pos_inf) begin
            result = 32'h7F800000;
        end else if (is_neg_inf) begin
            result = 32'h7FC00000;
        end else if (is_zero) begin
            result = 32'h00000000;
        end else if (sign == 1) begin
            // Sqrt(negative) = NaN
            result = 32'h7FC00000;
        end else begin    
            sqrt_exponent <= ((exponent - 8'd127) >> 1) + 8'd127;
            adjusted_mantissa <= {1'b1, mantissa};
            sqrt_mantissa = fixed_sqrt_result;
            if (exponent[0] == 0) begin
                // Odd Exponent Case (as bias is odd)
                product = (sqrt_mantissa * ROOT2);
                sqrt_mantissa = product[46:23] + 1'b1;
            end
            result = {1'b0, sqrt_exponent, sqrt_mantissa[22:0]};
        end
    end
endmodule
