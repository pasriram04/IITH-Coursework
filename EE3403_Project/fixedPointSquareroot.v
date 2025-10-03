`timescale 1ns / 1ps

module fixedPointSquareroot(
    input [22:0] Y,
    input clk,
    output wire [23:0] result
);
    // Input is 23-bit mantissa; the first bit of Y is assumed to be 1
    wire [23:0] Y_new = {1'b1, Y}; // Hidden Bit
    wire [5:0] Y_cap = Y[22:17];

    reg [8:0] R_LUT [0:63];
    reg [27:0] M_LUT [0:63];

    reg [8:0] R;
    reg [27:0] M;

    // Combinational signals
    wire [31:0] product = R * Y_new;
    wire [23:0] A_raw = product[31:8] - 24'h800000;
    wire [23:0] A = A_raw[23] ? (~A_raw + 1'b1) : A_raw; // Two's complement if negative

    wire [6:0] A2 = A[15:9];
    wire [6:0] A3 = A[8:2];
    wire [23:0] A2_square = (A2 * A2) >> 5;
    wire [23:0] A2_cube = (A2 * A2 * A2) >> 19;
    wire [23:0] A2_A3_prod = (A2 * A3) >> 12;

    wire [23:0] term1 = 24'h800000;
    wire [23:0] term2 = A_raw[23] ? (A >> 1) : ({2'b0, A[23:1]});
    wire [23:0] term3 = A2_square >> 3;
    wire [23:0] term4 = A2_A3_prod >> 2;
    wire [23:0] term5 = A2_cube >> 4;

    wire [23:0] B = A_raw[23]
        ? (term1 - term2 - term3 - term4 - term5)
        : (term1 + term2 - term3 - term4 + term5);

    wire [50:0] prod = M * B;

    always @(posedge clk) begin
        $readmemb("lutR.mem", R_LUT);
        $readmemb("lutM.mem", M_LUT);

        R <= R_LUT[Y_cap];
        M <= M_LUT[Y_cap];
    end
    assign result = prod[50:27];
endmodule
