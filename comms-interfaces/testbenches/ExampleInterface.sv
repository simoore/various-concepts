`timescale 1ns / 1ps

///////////////////////////////////////////////////////////////////////////////
// We can use an interface like a struct type.
///////////////////////////////////////////////////////////////////////////////

module and4 (
    input [3:0] a,
    input [3:0] b,
    output [3:0] y
);
    
    assign y = a & b; 

endmodule

interface and_if;
    logic [3:0] a;
    logic [3:0] b;
    logic [3:0] y;
endinterface

module ExampleInterface;

    and_if aif();
    
    and4 dut (.a(aif.a), .b(aif.b), .y(aif.y));
    
    initial begin
        aif.a = 4'b0100;
        aif.b = 4'b1100;
        #10;
        $display("a : %b , b : %b and y : %b",aif.a, aif.b, aif.y );
    end

endmodule
