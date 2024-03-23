localparam DW = 32;                  // Width of AXI data path.
localparam BW = 12;                  // Assign a 4kB block to memory map this module.
localparam AW = 32;                  // AXI address width.
localparam SW = DW >> 3;             // The strobe signal has 1 bit for each byte in the data path.

// These registers can only be read by software over the AXI interface.
typedef struct packed {
    logic [DW-1:0] testRead; // 'hF428A315;
    logic [DW-1:0] version; // = {8'h0, 8'h0, 8'h1, 8'h1};
} Statuses;

// Number of 32 bit words in the status structure.
localparam STATUS_SIZE = $bits(Statuses) >> 5;

// These registers can only be modified by software over the AXI interface.
typedef struct packed {
    logic [DW-1:0] testWrite;   // 0
    logic [DW-1:0] startNonce;  // 0
} Configs;

// Number of 32 bit words in the config structure.
localparam CONFIG_SIZE = $bits(Configs) >> 5;

// These registers can be written to by both the software and the FPGA logic.
typedef struct packed {
    // Setting a bit to 1 starts a certain action, and the bit is cleared when finished.
    // Setting bit 0 to 1 starts the mining process.
    // Setting bit 1 to 1 computes the hash of the block.
    logic [DW-1:0] testControl; // Default = 0;
} Control;

// Number of 32 bit words in the control structure.
localparam CONTROL_SIZE = $bits(Control) >> 5;

/**********************************************************************************************************************
* Miner Registers
* ---------------
* Provides and AXI Lite Slave Interface for the processor to read and write to control registers in the miner.
* This interface is heavily based on https://github.com/ZipCPU/wb2axip/blob/master/rtl/easyaxil.v.
/*********************************************************************************************************************/
module MinerRegisters (
    input logic clk,            // System clock (ADC 125MHz clock)
    input logic rstn,           // Active low reset
    input Statuses statusesIn,  // Statuses registers from the FPGA logic to the processor.
    output Configs configsOut,  // Config registers from the processor to the FPGA logic.
    input logic controlWrite,   // 1 when the FPGA logic wants to write to the control registers. 
    input Control controlIn,    // Desired value to write to the control registers.
    output Control controlOut,  // The actual value of the control registers.
    AxiLiteIF.Slave axi
);

    /******************************************************************************************************************
    * Local Constants
    ******************************************************************************************************************/  

    localparam int START_CONFIG = STATUS_SIZE;
    localparam int START_CONTROL = START_CONFIG + CONFIG_SIZE;
    localparam int END_REGISTERS = START_CONTROL + CONTROL_SIZE;

    /******************************************************************************************************************
    * Local Types
    ******************************************************************************************************************/  
    
    // Collection of local registers for the AXI interface.
    typedef struct {
        logic bvalid;
        logic readValid;
        logic awready;
        logic [DW-1:0] readData;
    } AxiRegisters;
    
    // Collection of local signals for the AXI interface.
    typedef struct {
        logic readReady;
    } AxiSignals;

    /******************************************************************************************************************
    * Local Signals and Registers
    ******************************************************************************************************************/           
    
    AxiSignals axiSig;
    AxiRegisters axiReg = '{default: 0};
    
    logic [BW-4:0] readIndex;
    logic [BW-4:0] writeIndex;
    logic [STATUS_SIZE-1:0][DW-1:0] statusesArr;
    logic [CONFIG_SIZE-1:0][DW-1:0] configsArr = '{default: 0};
    logic [CONTROL_SIZE-1:0][DW-1:0] controlArr = '{default: 0};
    
    /******************************************************************************************************************
    * AXI Lite Slave Interface to Processor
    * -------------------------------------
    * Based on https://github.com/ZipCPU/wb2axip/blob/master/rtl/easyaxil.v
    ******************************************************************************************************************/
    
    assign axi.rresp = 2'b00;
    assign axi.bresp = 2'b00;
    
    always_ff @(posedge clk) begin
        if (!rstn) begin
            axiReg.awready <= 1'b0;
        end else begin
            axiReg.awready <= !axiReg.awready & (axi.awvalid & axi.wvalid) & (!axi.bvalid | axi.bready);
        end
    end
    
    assign axi.awready = axiReg.awready;
    assign axi.wready = axiReg.awready;
    
    always_ff @(posedge clk) begin
        if (!rstn) begin
            axiReg.bvalid <= 0;
        end else if (axiReg.awready) begin
            axiReg.bvalid <= 1;
        end else if (axi.bready) begin
            axiReg.bvalid <= 0;
        end
    end
    
    assign axi.bvalid = axiReg.bvalid;
    assign axi.arready = !axi.rvalid;
    assign axiSig.readReady = axi.arvalid & axi.arready;
        
    always_ff @(posedge clk) begin
        if (!rstn) begin
            axiReg.readValid <= 1'b0;
        end else if (axiSig.readReady) begin
            axiReg.readValid <= 1'b1;
        end else if (axi.rready) begin
            axiReg.readValid <= 1'b0;
        end
    end
    
    assign axi.rvalid = axiReg.readValid;
    assign axi.rdata = axiReg.readData;
    
    /******************************************************************************************************************
    * Register Logic
    * --------------
    * Reads and writes the data from the AXI bus to/from the control register.
    ******************************************************************************************************************/
    
    // The strobe indicates which bytes on the bus are to be written to memory and which are not.
    function [DW-1:0] applyWstrb;
        input	[DW-1:0]    priorData;
        input	[DW-1:0]    newData;
        input	[SW-1:0]	wstrb;
    
        for (int k = 0; k < SW; k = k + 1) begin
            applyWstrb[k*8 +: 8] = wstrb[k] ? newData[k*8 +: 8] : priorData[k*8 +: 8];
        end
    endfunction
       
    assign statusesArr = statusesIn;
    assign configsOut = configsArr;
    assign controlOut = controlArr;
    assign readIndex = axi.araddr[BW-1:3];
    assign writeIndex = axi.awaddr[BW-1:3];
    
    // Write registers.
    always_ff @(posedge clk) begin
        if (axiReg.awready) begin
            if (writeIndex >= START_CONFIG && writeIndex < START_CONTROL) begin
                configsArr[writeIndex] <= applyWstrb(configsArr[writeIndex], axi.wdata, axi.wstrb);
            end 
        end
       
        if (controlWrite) begin
            controlArr <= controlIn;
        end else if (axiReg.awready) begin
            if (writeIndex >= START_CONTROL && writeIndex < END_REGISTERS) begin
                controlArr[writeIndex] <= applyWstrb(controlArr[writeIndex], axi.wdata, axi.wstrb);
            end
        end
    end
    
    // Read registers.
    always_ff @(posedge clk) begin
        if (readIndex < START_CONFIG) begin
            axiReg.readData <= statusesArr[readIndex];
        end else if (writeIndex >= START_CONFIG && writeIndex < START_CONTROL) begin
            axiReg.readData <= configsArr[readIndex];
        end else if (writeIndex >= START_CONTROL && writeIndex < END_REGISTERS) begin
            axiReg.readData <= controlArr[readIndex];
        end else begin  
            axiReg.readData <= axi.araddr;
        end 
    end
    
endmodule : MinerRegisters