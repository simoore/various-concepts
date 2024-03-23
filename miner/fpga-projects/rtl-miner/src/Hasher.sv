`timescale 1ns / 1ps

typedef struct {
    logic firstBlock;
    logic lastBlock;
    logic [31:0] block [0:15];
} HasherBlock;

interface HasherPort (
    input logic clk
);
    logic rst;
    
    logic validIn;
    logic readyOut;
    HasherBlock dataIn;
    
    logic readyHashIn;
    logic validHashOut;
    logic [31:0] hashOut [0:7];
    
    modport Main (
        input clk,
        
        input rst,
        input validIn,
        output readyOut,
        input dataIn,
        
        input readyHashIn,
        output validHashOut,
        output hashOut
    );
    
endinterface

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The hasher takes in a set of blocks and computes the hash for the data in the blocks. It will restart the process
// when it receives the firstBlockIn signal. And it will output the hash after it receives the lastBlockIn signal. 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module Hasher(
    HasherPort.Main port
);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TYPES
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef enum {
        Idle = 0,           // Can accept a new block of data on the input.
        LastRound = 1,      // If we are exeuting the last round of the SHA256 compression.
        HashingBlock = 2,   // While we are computing the rounds of the SHA265 compression.
        UpdateHash = 3,     // We are adding the result of the compression to the previosu hash.
        TransferOut = 4     // After we receive the last block to digest, this state indicates the hash should be output.
    } State;
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CONSTANTS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    localparam logic [31:0] ROUNDING_CONSTANTS[0:63] = {
        32'h428a2f98, 32'h71374491, 32'hb5c0fbcf, 32'he9b5dba5, 32'h3956c25b, 32'h59f111f1, 32'h923f82a4, 32'hab1c5ed5,
        32'hd807aa98, 32'h12835b01, 32'h243185be, 32'h550c7dc3, 32'h72be5d74, 32'h80deb1fe, 32'h9bdc06a7, 32'hc19bf174,
        32'he49b69c1, 32'hefbe4786, 32'h0fc19dc6, 32'h240ca1cc, 32'h2de92c6f, 32'h4a7484aa, 32'h5cb0a9dc, 32'h76f988da,
        32'h983e5152, 32'ha831c66d, 32'hb00327c8, 32'hbf597fc7, 32'hc6e00bf3, 32'hd5a79147, 32'h06ca6351, 32'h14292967,
        32'h27b70a85, 32'h2e1b2138, 32'h4d2c6dfc, 32'h53380d13, 32'h650a7354, 32'h766a0abb, 32'h81c2c92e, 32'h92722c85,
        32'ha2bfe8a1, 32'ha81a664b, 32'hc24b8b70, 32'hc76c51a3, 32'hd192e819, 32'hd6990624, 32'hf40e3585, 32'h106aa070,
        32'h19a4c116, 32'h1e376c08, 32'h2748774c, 32'h34b0bcb5, 32'h391c0cb3, 32'h4ed8aa4a, 32'h5b9cca4f, 32'h682e6ff3,
        32'h748f82ee, 32'h78a5636f, 32'h84c87814, 32'h8cc70208, 32'h90befffa, 32'ha4506ceb, 32'hbef9a3f7, 32'hc67178f2
    };
        
    localparam WorkingVars INITIAL_VARS = '{
        a: 32'h6a09e667,
        b: 32'hbb67ae85,
        c: 32'h3c6ef372,
        d: 32'ha54ff53a,
        e: 32'h510e527f,
        f: 32'h9b05688c,
        g: 32'h1f83d9ab,
        h: 32'h5be0cd19
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // LOCAL SIGNALS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    logic transferIn;
    logic startHashing;
    logic stalled;
    logic transferHash;
    logic nextValidHash;
    State nextState;

    logic ready = 0;
    logic [7:0] j = 0;
    State state = Idle;
    WorkingVars hashLocal = '{default: 0};
    HasherBlock data = '{default: 0};
    logic validHash = 0;
    logic [31:0] hash [0:7] = '{default: 0};
    
    Sha256Port sha256Port(port.clk);
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // STREAM INPUT BUFFER
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    assign port.readyOut = ready;
    assign transferIn = port.validIn & ready;
    
    // Ready signal for the block input stream.
    always_ff @(posedge port.clk) begin
        if (port.rst) begin
            ready <= 0;
        end else begin
            ready <= (state == Idle) & !transferIn;
        end
    end
    
    // Buffer the input when input transfer condition is true.
    always_ff @(posedge port.clk) begin
        if (transferIn) begin
            data <= port.dataIn;
        end
    end
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // SHA256 ROUND COUNTER
    // --------------------
    // Counts out the number of rounds required to digest a block.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    always_ff @(posedge port.clk) begin
        if (state == Idle) begin
            j <= 0;
        end else if (state == HashingBlock) begin
            j <= j + 1;
        end
    end
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // hashLocal stores the result of the hash after each block has been digested.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // On the first block in the message, the hash is reset to an initial value. Then it is updated after each block
    // is digested.
    always_ff @(posedge port.clk) begin
        if (port.rst) begin
            hashLocal <= '{default: 0};
        end else if (transferIn & port.dataIn.firstBlock) begin
            hashLocal <= INITIAL_VARS;
        end else if (state == UpdateHash) begin
            hashLocal.a <= hashLocal.a + sha256Port.varsOut.a;
            hashLocal.b <= hashLocal.b + sha256Port.varsOut.b;
            hashLocal.c <= hashLocal.c + sha256Port.varsOut.c;
            hashLocal.d <= hashLocal.d + sha256Port.varsOut.d;
            hashLocal.e <= hashLocal.e + sha256Port.varsOut.e;
            hashLocal.f <= hashLocal.f + sha256Port.varsOut.f;
            hashLocal.g <= hashLocal.g + sha256Port.varsOut.g;
            hashLocal.h <= hashLocal.h + sha256Port.varsOut.h;
        end
    end
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // VALID HASH OUT REGISTER
    // -----------------------
    // These are a set registers that implement an M_AXIS interface.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // If (validOut and !readyIn) then the stream is stalled and the registers need to hold their value.
    assign stalled = validHash && !port.readyHashIn;
    
    // This indicates a transfer of data has occurred on the AXIS interface.
    assign transferHash = validHash && port.readyHashIn;
    
    // The data is output for one clock cycle is we have finished hashing, its the last block and no transfer has 
    // already occured.
    assign nextValidHash = (state == TransferOut) && data.lastBlock && !transferHash;
    
    // Map the valid hash signal to the port.
    assign port.validHashOut = validHash;
    
    // Map the hash to the port.
    assign port.hashOut = hash;
    
    always @(posedge port.clk) begin
        if (port.rst) begin
            hash <= '{default: 0};
            validHash <= 0;
        end else if (!stalled) begin
            if (nextValidHash) begin
                hash[0] <= hashLocal.a;
                hash[1] <= hashLocal.b;
                hash[2] <= hashLocal.c;
                hash[3] <= hashLocal.d;
                hash[4] <= hashLocal.e;
                hash[5] <= hashLocal.f;
                hash[6] <= hashLocal.g;
                hash[7] <= hashLocal.h;
            end
            validHash <= nextValidHash;
        end
    end
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // SHA256 COMPRESSION
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
    assign sha256Port.wIn = (j == 0) ? data.block : sha256Port.wOut;
    assign sha256Port.varsIn = (j == 0) ? hashLocal : sha256Port.varsOut;
    assign sha256Port.k = ROUNDING_CONSTANTS[j];
    
    Sha256Compression compression(sha256Port);
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // HASHER STATE MACHINE
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    always_ff @(posedge port.clk) begin
        state <= nextState;
    end
    
    always_comb begin
        nextState = state;
        case (state)
        Idle:
            if (transferIn) begin
                nextState = HashingBlock;
            end
        HashingBlock:
            if (j == 62) begin
                nextState = LastRound; 
            end
        LastRound:
            nextState = UpdateHash;
        UpdateHash:
            if (data.lastBlock) begin
                nextState = TransferOut;
            end else begin
                nextState = Idle;
            end
        TransferOut: 
            if (transferHash) begin
                nextState = Idle;
            end
        endcase
    end

endmodule : Hasher
