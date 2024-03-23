// Define an interface for the FIFO
interface fifo_if;

    logic clk; 
    logic rst;
    logic rd;
    logic wr;         
    logic full;
    logic empty;    
    logic [7:0] data_in;     
    logic [7:0] data_out;     
                      
endinterface


module Fifo #(
    FIFO_SIZE = 32,
    DATA_SIZE = 32
) (
    // Clock and reset.
    input logic clk,
    input logic rst,
    
    // Data input port.
    input logic wr,
    output logic full,
    input logic [DATA_SIZE-1:0] din,
    
    // Data output port.
    input logic rd,
    output logic empty,    
    output logic [DATA_SIZE-1:0] dout  
);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // LOCAL SIGNALS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    logic push;     // If data is going to be written to the FIFO
    logic pop;      // If data is going to be read to the FIFO             
    logic [3:0] wptr = 0;   // Pointer for write operation
    logic [3:0] rptr = 0;   // Pointer for read operation
    logic [4:0] cnt = 0;    // Counter for tracking the number of elements in the FIFO
    logic [DATA_SIZE-1:0] mem [FIFO_SIZE-1:0];      // Memory array to store data
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FIFO LOGIC
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    assign push = wr && !full;
    assign pop = rd && !empty;
    assign empty = cnt == 0;
    assign full  = cnt == FIFO_SIZE;
    
    always @(posedge clk) begin

        // Write data to the FIFO if it's not full.
        if (rst) begin
            wptr <= 0;
        end else if (push) begin
            mem[wptr] <= din;
            wptr <= wptr + 1;
        end
        
        // Read data from the FIFO if it's not empty.
        if (rst) begin
            rptr <= 0;
        end else if (pop) begin
            dout <= mem[rptr];
            rptr <= rptr + 1;
        end
        
        if (rst) begin
            cnt <= 0;
        end else if (pop & ~push) begin
            cnt <= cnt - 1;
        end else if (push & ~pop) begin
            cnt <= cnt + 1;
        end
    end
    
endmodule
