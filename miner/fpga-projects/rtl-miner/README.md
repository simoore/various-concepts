# RTL Miner

The software writes a block to memory for hashing or mining. The software performs the padding required by the SHA256
algorithm as this only needs to be performed once and is a simple operation. This block is preceded by a descriptor 
that includes:

* The size of the block
* A block ID for when reporting results
* A starting nonce if mining
* An action including if we are mining or hashing. There are a number of debug actions as well.

When the block is written to memory, a flag in the DMA descriptor is set and the FPGA processes the block. Then the 
software awaits the result which includes:

* The hash and nonce
* Any debug information or statistics

## Generate Vivado Project and Compile Bitstream

* Execute generate.sh to build the project and compile the bitstream.
* You can use Vivado when appropriate to modify 

### Hardware Setup and Export

* The board files for the Cora Z7-07S development board can be downloaded via the Vivado Store which is found in
  tools menu. The board is the Cora Z7-07S.
* Route the `DDR` and `FIXED_IO` signals from the Zynq processor to the top-level module. Note that the 
  `FIXED_IO_mio` signal needs to be assigned to a signal just called `mio` at the top level for the automatically
  generated constraint files to work.
* Once the FPGA design is completed

## Links

### FPGA Development

* [Building a basic AXI Master](https://zipcpu.com/blog/2020/03/23/wbm2axisp.html)
* [Building a custom yet functional AXI-lite slave](https://zipcpu.com/blog/2019/01/12/demoaxilite.html)
* [WB2AXIP: Bus interconnects, bridges, and other components](https://github.com/ZipCPU/wb2axip)

### Bitcoint Mining

* [Understanding the Bitcoin Blockchain Header](https://medium.com/fcats-blockchain-incubator/understanding-the-bitcoin-blockchain-header-a2b0db06b515)

### SHA256 Algorithm

* [SHA256 Wikipedia](https://en.wikipedia.org/wiki/SHA-2)
* [SHA256 FPGA implementation](https://github.com/secworks/sha256)
* [Open-Source FPGA Bitcoin Miner](https://github.com/progranism/Open-Source-FPGA-Bitcoin-Miner)
* [US Secure Hash Algorithms (SHA and HMAC-SHA)](https://www.rfc-editor.org/rfc/rfc4634)
* [Descriptions of SHA-256, SHA-384, and SHA-512](https://eips.ethereum.org/assets/eip-2680/sha256-384-512.pdf)

### Project Setup Examples

* [Xilinx Vivado setup for source control](https://github.com/jhallen/vivado_setup)

### Verification

* [Chip Verify: UVM Tutorial](https://www.chipverify.com/uvm/uvm-tutorial)
