`timescale 1ns / 1ps

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write a function capable of generating a square waveform with the help of period(in nSec) and duty cycle(0 to 1). 
// The phase difference is assumed to be 0 for the entire system operation. Verify function behavior by generating 
// waveform for the signal clk with period: 40 nsec and duty cycle: 0.4
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module Assignment24;

logic clk;

task clkgen(input real duty, input real period);  
    real ton = duty * period;
    real toff = (1 - duty) * period;
    while (1) begin
        clk = 0;
        #toff;
        clk = 1;
        #ton;
    end
endtask

initial begin
    clkgen(0.4, 40);
end

endmodule
