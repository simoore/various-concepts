`timescale 1ns / 1ps

module TestHasher;

logic clk;
HasherPort hasherPort(clk);
Hasher dut(.port(hasherPort));

always begin
    #1 clk = !clk;
end

initial begin
    clk = 0;
    hasherPort.rst = 1;
    hasherPort.validIn = 0;
    hasherPort.readyHashIn = 0;
    
    hasherPort.dataIn.firstBlock = 1;
    hasherPort.dataIn.lastBlock = 1;
    hasherPort.dataIn.block = { 32'h61626380, 32'h0, 32'h0, 32'h0, 32'h0, 32'h0, 32'h0, 32'h0,
        32'h0, 32'h0, 32'h0, 32'h0, 32'h0, 32'h0, 32'h0, 32'h18 };
        
    @(posedge clk);
    hasherPort.rst = 0;
    
    @(posedge clk);
    hasherPort.validIn = 1;
    @(posedge clk);
    hasherPort.validIn = 0;
    
    for (int i = 0; i < 100; i++) begin
        @(posedge clk);
        if (i == 90) begin
            hasherPort.readyHashIn = 1;
        end
    end
    
    hasherPort.validIn = 0;
    hasherPort.dataIn.firstBlock = 1;
    hasherPort.dataIn.lastBlock = 0;
    hasherPort.dataIn.block = { 
        32'h61626364, 32'h62636465, 32'h63646566, 32'h64656667, 32'h65666768, 32'h66676869, 32'h6768696a, 32'h68696a6b,
        32'h696a6b6c, 32'h6a6b6c6d, 32'h6b6c6d6e, 32'h6c6d6e6f, 32'h6d6e6f70, 32'h6e6f7071, 32'h80000000, 32'h00000000 };
    
    @(posedge clk);
    hasherPort.validIn = 1;
    @(posedge clk);
    hasherPort.validIn = 0;
    
    for (int i = 0; i < 100; i++) begin
        @(posedge clk);
    end
    
    hasherPort.dataIn.firstBlock = 0;
    hasherPort.dataIn.lastBlock = 1;
    hasherPort.dataIn.block = { 
        32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000,
        32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, 32'h000001c0 };
   
    @(posedge clk);
    hasherPort.validIn = 1;
    @(posedge clk);
    hasherPort.validIn = 0;
    
    for (int i = 0; i < 100; i++) begin
        @(posedge clk);
    end
    
    $finish;
end

endmodule