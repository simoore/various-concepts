module fifo(
    input logic wreq,
    input logic rreq,
    input logic clk,
    input logic rst,
    input logic [7:0] wdata,
    output logic [7:0] rdata,
    output logic f,
    output logic e
);
endmodule
