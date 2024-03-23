/**********************************************************************************************************************
* AXI Stream Bus Interface
* ------------------------
* Groups the signals that make up an AXI Stream bus.
**********************************************************************************************************************/ 
interface StreamIF;

    logic [31:0] tdata;
    logic [3:0]  tkeep;
    logic        tlast;
    logic        tready;
    logic        tvalid;
    
    modport Master (
        output tdata,
        output tkeep,
        output tlast,
        input  tready,
        output tvalid
    );
    
    modport Slave (
        input  tdata,
        input  tkeep,
        input  tlast,
        output tready,
        input  tvalid
    );

endinterface : StreamIF

/**********************************************************************************************************************
* This is the bus for an AXI Lite peripheral.
**********************************************************************************************************************/ 
interface AxiLiteIF;

    // axi write address channel
    logic [AW-1:0] awaddr;    // AXI write address
    logic [2:0]    awprot;    // AXI write protection type
    logic          awvalid;   // AXI write address valid
    logic          awready;   // AXI write ready
    
    // axi write data channel
    logic [DW-1:0] wdata;     // AXI write data
    logic [SW-1:0] wstrb;     // AXI write strobes
    logic          wvalid;    // AXI write valid
    logic          wready;    // AXI write ready
    
    // axi write response channel
    logic [1:0]    bresp;     // AXI write response
    logic          bvalid;    // AXI write response valid
    logic          bready;             // AXI write response ready
    
    // axi read address channel
    logic [AW-1:0] araddr;    // AXI read address
    logic [2:0]    arprot;    // AXI read protection type
    logic          arvalid;   // AXI read address valid
    logic          arready;   // AXI read address ready
    
    // axi read data channel
    logic [DW-1:0] rdata;     // AXI read data
    logic [1:0]    rresp;     // AXI read response
    logic          rvalid;    // AXI read response valid
    logic          rready;    // AXI read response readye
    
    modport Slave (
        // axi write address channel
        input  awaddr,    // AXI write address
        input  awprot,    // AXI write protection type
        input  awvalid,   // AXI write address valid
        output awready,   // AXI write ready
        
        // axi write data channel
        input  wdata,     // AXI write data
        input  wstrb,     // AXI write strobes
        input  wvalid,    // AXI write valid
        output wready,    // AXI write ready
        
        // axi write response channel
        output bresp,     // AXI write response
        output bvalid,    // AXI write response valid
        input  bready,    // AXI write response ready
        
        // axi read address channel
        input  araddr,    // AXI read address
        input  arprot,    // AXI read protection type
        input  arvalid,   // AXI read address valid
        output arready,   // AXI read address ready
        
        // axi read data channel
        output rdata,     // AXI read data
        output rresp,     // AXI read response
        output rvalid,    // AXI read response valid
        input  rready     // AXI read response readye
    );
    
    modport Master (
        // axi write address channel
        output awaddr,    // AXI write address
        output awprot,    // AXI write protection type
        output awvalid,   // AXI write address valid
        input  awready,   // AXI write ready
        
        // axi write data channel
        output wdata,     // AXI write data
        output wstrb,     // AXI write strobes
        output wvalid,    // AXI write valid
        input  wready,    // AXI write ready
        
        // axi write response channel
        input  bresp,     // AXI write response
        input  bvalid,    // AXI write response valid
        output bready,    // AXI write response ready
        
        // axi read address channel
        output araddr,    // AXI read address
        output arprot,    // AXI read protection type
        output arvalid,   // AXI read address valid
        input  arready,   // AXI read address ready
        
        // axi read data channel
        input  rdata,     // AXI read data
        input  rresp,     // AXI read response
        input  rvalid,    // AXI read response valid
        output rready     // AXI read response readye
    );
    
endinterface : AxiLiteIF

/**********************************************************************************************************************
* Processor Inferface
* -------------------
* This interface routes the DDR and FIXED_IO signals to the top-level module.
**********************************************************************************************************************/
interface ProcessorIF(
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
    inout logic [53:0]    FIXED_IO_mio,
    inout logic           FIXED_IO_ps_clk,
    inout logic           FIXED_IO_ps_porb,
    inout logic           FIXED_IO_ps_srstb
);
    
    modport Main (
        inout DDR_addr,
        inout DDR_ba,
        inout DDR_cas_n,
        inout DDR_ck_n,
        inout DDR_ck_p,
        inout DDR_cke,
        inout DDR_cs_n,
        inout DDR_dm,
        inout DDR_dq,
        inout DDR_dqs_n,
        inout DDR_dqs_p,
        inout DDR_odt,
        inout DDR_ras_n,
        inout DDR_reset_n,
        inout DDR_we_n,
    
        inout FIXED_IO_ddr_vrn,
        inout FIXED_IO_ddr_vrp,
        inout FIXED_IO_mio,
        inout FIXED_IO_ps_clk,
        inout FIXED_IO_ps_porb,
        inout FIXED_IO_ps_srstb
    );
  
endinterface : ProcessorIF

/**********************************************************************************************************************
* Processing System
* -----------------
* This module encaptulates the processing system block design.
**********************************************************************************************************************/ 
module MinerProcessingSystem (
    output logic        clk,
    output logic        rstn,
    StreamIF.Master     dmaMaster,
    StreamIF.Master     dmaControl,
    StreamIF.Slave      dmaSlave,
    StreamIF.Slave      dmaStatus,
    AxiLiteIF.Master    controlRegistersAxi,
    ProcessorIF.Main    processorIF
);
    
    /******************************************************************************************************************
    * Processing System
    ******************************************************************************************************************/ 
    
    ProcessingSystem processingSystem (
       
        // 250MHz clock and associated active low reset.  
        .FCLK_CLK0_0            (clk),
        .FCLK_RESET0_N_0        (rstn),

        // Interface to write to the miner's control registers.
        .M00_AXI_0_araddr       (controlRegistersAxi.araddr),
        .M00_AXI_0_arprot       (controlRegistersAxi.arprot),
        .M00_AXI_0_arready      (controlRegistersAxi.arready),
        .M00_AXI_0_arvalid      (controlRegistersAxi.arvalid),
        .M00_AXI_0_awaddr       (controlRegistersAxi.awaddr),
        .M00_AXI_0_awprot       (controlRegistersAxi.awprot),
        .M00_AXI_0_awready      (controlRegistersAxi.awready),
        .M00_AXI_0_awvalid      (controlRegistersAxi.awvalid),
        .M00_AXI_0_bready       (controlRegistersAxi.bready),
        .M00_AXI_0_bresp        (controlRegistersAxi.bresp),
        .M00_AXI_0_bvalid       (controlRegistersAxi.bvalid),
        .M00_AXI_0_rdata        (controlRegistersAxi.rdata),
        .M00_AXI_0_rready       (controlRegistersAxi.rready),
        .M00_AXI_0_rresp        (controlRegistersAxi.rresp),
        .M00_AXI_0_rvalid       (controlRegistersAxi.rvalid),
        .M00_AXI_0_wdata        (controlRegistersAxi.wdata),
        .M00_AXI_0_wready       (controlRegistersAxi.wready),
        .M00_AXI_0_wstrb        (controlRegistersAxi.wstrb),
        .M00_AXI_0_wvalid       (controlRegistersAxi.wvalid),
        
        // Interface for the DMA to read and write to memory.
        .M_AXIS_CNTRL_0_tdata   (dmaControl.tdata),
        .M_AXIS_CNTRL_0_tkeep   (dmaControl.tkeep),
        .M_AXIS_CNTRL_0_tlast   (dmaControl.tlast),
        .M_AXIS_CNTRL_0_tready  (dmaControl.tready),
        .M_AXIS_CNTRL_0_tvalid  (dmaControl.tvalid),
        .M_AXIS_MM2S_0_tdata    (dmaMaster.tdata),
        .M_AXIS_MM2S_0_tkeep    (dmaMaster.tkeep),
        .M_AXIS_MM2S_0_tlast    (dmaMaster.tlast),
        .M_AXIS_MM2S_0_tready   (dmaMaster.tready),
        .M_AXIS_MM2S_0_tvalid   (dmaMaster.tvalid),
        .S_AXIS_S2MM_0_tdata    (dmaSlave.tdata),
        .S_AXIS_S2MM_0_tkeep    (dmaSlave.tkeep),
        .S_AXIS_S2MM_0_tlast    (dmaSlave.tlast),
        .S_AXIS_S2MM_0_tready   (dmaSlave.tready),
        .S_AXIS_S2MM_0_tvalid   (dmaSlave.tvalid),
        .S_AXIS_STS_0_tdata     (dmaStatus.tdata),
        .S_AXIS_STS_0_tkeep     (dmaStatus.tkeep),
        .S_AXIS_STS_0_tlast     (dmaStatus.tlast),
        .S_AXIS_STS_0_tready    (dmaStatus.tready),
        .S_AXIS_STS_0_tvalid    (dmaStatus.tvalid),
        
        // The DDR interface.
        .DDR_addr               (processorIF.DDR_addr),
        .DDR_ba                 (processorIF.DDR_ba),
        .DDR_cas_n              (processorIF.DDR_cas_n),
        .DDR_ck_n               (processorIF.DDR_ck_n),
        .DDR_ck_p               (processorIF.DDR_ck_p),
        .DDR_cke                (processorIF.DDR_cke),
        .DDR_cs_n               (processorIF.DDR_cs_n),
        .DDR_dm                 (processorIF.DDR_dm),
        .DDR_dq                 (processorIF.DDR_dq),
        .DDR_dqs_n              (processorIF.DDR_dqs_n),
        .DDR_dqs_p              (processorIF.DDR_dqs_p),
        .DDR_odt                (processorIF.DDR_odt),
        .DDR_ras_n              (processorIF.DDR_ras_n),
        .DDR_reset_n            (processorIF.DDR_reset_n),
        .DDR_we_n               (processorIF.DDR_we_n),
    
        // The FIXED_IO interface
        .FIXED_IO_ddr_vrn       (processorIF.FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp       (processorIF.FIXED_IO_ddr_vrp),
        .FIXED_IO_mio           (processorIF.FIXED_IO_mio),
        .FIXED_IO_ps_clk        (processorIF.FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb       (processorIF.FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb      (processorIF.FIXED_IO_ps_srstb)
    );
    
endmodule : MinerProcessingSystem
