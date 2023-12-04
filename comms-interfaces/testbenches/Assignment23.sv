`timescale 1ns / 1ps

module Assignment23;

logic clk;

initial begin
    clk = 0;
end

always begin
    #55.5555
    clk = ~clk;
end

endmodule
