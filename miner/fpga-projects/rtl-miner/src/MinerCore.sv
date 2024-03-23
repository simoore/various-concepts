/**********************************************************************************************************************
* MinerCore 
* ---------
* This is the top-level module in the miner which connects the control registers, the hashers, the processing
* system with its DMA stream, and some top-level operation logic.
**********************************************************************************************************************/ 
module MinerCore #( 
    parameter int N_HASHERS = 2 
) (
    inout logic [14:0]    DDR_addr,
    inout logic [2:0]     DDR_ba,
    inout logic           DDR_cas_n,
    inout logic           DDR_ck_n,
    inout logic           DDR_ck_p,
    inout logic           DDR_cke,
    inout logic           DDR_cs_n,
    inout logic [3:0]     DDR_dm,
    inout logic [31:0]    DDR_dq,
    inout logic [3:0]     DDR_dqs_n,
    inout logic [3:0]     DDR_dqs_p,
    inout logic           DDR_odt,
    inout logic           DDR_ras_n,
    inout logic           DDR_reset_n,
    inout logic           DDR_we_n,

    inout logic           FIXED_IO_ddr_vrn,
    inout logic           FIXED_IO_ddr_vrp,
    inout logic [53:0]    mio,
    inout logic           FIXED_IO_ps_clk,
    inout logic           FIXED_IO_ps_porb,
    inout logic           FIXED_IO_ps_srstb
);

    /******************************************************************************************************************
    * Local Signals
    * -------------
    ******************************************************************************************************************/ 

    logic clk;
    logic rstn;
    AxiLiteIF axiLiteIF();
    StreamIF dmaResult();
    StreamIF dmaBlock();
    StreamIF dmaControl();
    StreamIF dmaStatus();
    
    ProcessorIF processorIF(
        .DDR_addr               (DDR_addr),
        .DDR_ba                 (DDR_ba),
        .DDR_cas_n              (DDR_cas_n),
        .DDR_ck_n               (DDR_ck_n),
        .DDR_ck_p               (DDR_ck_p),
        .DDR_cke                (DDR_cke),
        .DDR_cs_n               (DDR_cs_n),
        .DDR_dm                 (DDR_dm),
        .DDR_dq                 (DDR_dq),
        .DDR_dqs_n              (DDR_dqs_n),
        .DDR_dqs_p              (DDR_dqs_p),
        .DDR_odt                (DDR_odt),
        .DDR_ras_n              (DDR_ras_n),
        .DDR_reset_n            (DDR_reset_n),
        .DDR_we_n               (DDR_we_n),
        .FIXED_IO_ddr_vrn       (FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp       (FIXED_IO_ddr_vrp),
        .FIXED_IO_mio           (mio),
        .FIXED_IO_ps_clk        (FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb       (FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb      (FIXED_IO_ps_srstb)
    );
    
    /******************************************************************************************************************
    * Miner Registers
    * ---------------
    * I don't think this module is necessary. It is just here as an example for memory-mapped registers using an
    * AXILite interface.
    ******************************************************************************************************************/ 
    
    Statuses statuses = '{ testRead: 'hF428A315, version: 'h00000001 };
    
    MinerRegisters minerRegisters(
        .clk            (clk),
        .rstn           (rstn),
        .statusesIn     (statuses),         
        .configsOut     (),
        .controlWrite   (1'h0),        
        .controlIn      (0),         
        .controlOut     (),
        .axi            (axiLiteIF)          
    );
    
    /******************************************************************************************************************
    * The Hashers
    * -----------
    * These guys do all the work.
    ******************************************************************************************************************/ 
    
    generate for (genvar i = 0; i < N_HASHERS; i++) begin
        // Add a wrapper class around the hasher to modify the nonce
        HasherPort hasherPort(clk);
        Hasher hasher (hasherPort);
    end endgenerate
    
    /******************************************************************************************************************
    * Miner DMA Operator
    ******************************************************************************************************************/ 
    
    assign dmaResult.tdata = dmaBlock.tdata;
    assign dmaResult.tlast = dmaBlock.tlast;
    assign dmaResult.tvalid = dmaBlock.tvalid;
    assign dmaResult.tkeep = dmaBlock.tkeep;
    assign dmaBlock.tready = dmaResult.tready;
    
    /******************************************************************************************************************
    * Processing System
    * -----------------
    ******************************************************************************************************************/ 
    
    MinerProcessingSystem processingSystem (
        .clk                    (clk),
        .rstn                   (rstn),
        .dmaMaster              (dmaBlock),
        .dmaControl             (dmaControl),
        .dmaSlave               (dmaResult),
        .dmaStatus              (dmaStatus),
        .controlRegistersAxi    (axiLiteIF),
        .processorIF            (processorIF)
    );
    
endmodule : MinerCore
