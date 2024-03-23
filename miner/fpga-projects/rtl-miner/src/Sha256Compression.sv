typedef logic [31:0] uint32_t;

typedef struct {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint32_t f;
    uint32_t g;
    uint32_t h;
} WorkingVars;

interface Sha256Port(
    input logic clk
);
    uint32_t k;
    uint32_t wIn [0:15];
    WorkingVars varsIn;

    uint32_t wOut [0:15];
    WorkingVars varsOut;

    modport Main (
        input clk,
        input k,
        input wIn,
        input varsIn,
    
        output wOut,
        output varsOut
    );
endinterface : Sha256Port

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This computes one iteration of the compression algorithm used in SHA256. Feed-in the message schedule array `wIn`
// and working variables `workingVarsIn` from the previous iteration, along with the appropriate rounding constant
// `k`. After one clock cycle, the working variables and the message schedule array for this iteration are 
// output on `workingVarsOut` and `wOut`.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
module Sha256Compression(Sha256Port.Main port);

    function uint32_t calcCh(input uint32_t x, input uint32_t y, input uint32_t z);
        return (x & y) ^ (~x & z);
    endfunction
    
    function uint32_t calcMaj(input uint32_t x, input uint32_t y, input uint32_t z);
        return (x & y) ^ (x & z) ^ (y & z);
    endfunction

    function uint32_t calcBigSigma0(input uint32_t x);
        return {x[1:0], x[31:2]} ^ {x[12:0], x[31:13]} ^ {x[21:0], x[31:22]};
    endfunction

    function uint32_t calcBigSigma1(input uint32_t x);
        return {x[5:0], x[31:6]} ^ {x[10:0], x[31:11]} ^ {x[24:0], x[31:25]};
    endfunction
    
    function uint32_t calcSmallSigma0(input uint32_t x);
        return {x[6:0], x[31:7]} ^ {x[17:0], x[31:18]} ^ {3'h0, x[31:3]};
    endfunction

    function uint32_t calcSmallSigma1(input uint32_t x);
        return {x[16:0], x[31:17]} ^ {x[18:0], x[31:19]} ^ {10'h0, x[31:10]};
    endfunction

    function uint32_t calcT1(input uint32_t e, input uint32_t f, input uint32_t g, input uint32_t h, 
            input uint32_t k, input uint32_t w);
        return h + calcBigSigma1(e) + calcCh(e, f, g) + k + w;
    endfunction

    uint32_t t1;
    uint32_t t2;

    assign t1 = calcT1(port.varsIn.e, port.varsIn.f, port.varsIn.g, port.varsIn.h, port.k, port.wIn[0]); 
    assign t2 = calcBigSigma0(port.varsIn.a) + calcMaj(port.varsIn.a, port.varsIn.b, port.varsIn.c);
    
    always_ff @(posedge port.clk) begin
        port.wOut[0:14] <= port.wIn[1:15];
        port.wOut[15] <= port.wIn[0] + calcSmallSigma0(port.wIn[1]) + port.wIn[9] + calcSmallSigma1(port.wIn[14]);
    
        port.varsOut.a <= t1 + t2;
        port.varsOut.b <= port.varsIn.a;
        port.varsOut.c <= port.varsIn.b;
        port.varsOut.d <= port.varsIn.c;
        port.varsOut.e <= port.varsIn.d + t1;
        port.varsOut.f <= port.varsIn.e;
        port.varsOut.g <= port.varsIn.f;
        port.varsOut.h <= port.varsIn.g;
    end

endmodule
