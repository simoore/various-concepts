/* 
Find link.x in the target directory to see what default values you can override 
It appears that one of your sections needs to be called RAM.
*/

MEMORY
{
    ITCMRAM (xrw)   : ORIGIN = 0x00000000, LENGTH = 64K
    RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
    RAM_D1 (xrw)    : ORIGIN = 0x24000000, LENGTH = 512K
    RAM_D2 (xrw)    : ORIGIN = 0x30000000, LENGTH = 288K
    RAM_D3 (xrw)    : ORIGIN = 0x38000000, LENGTH = 64K
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 2048K
}

SECTIONS
{
    /*
     * Section in RAM_D2 that is used for buffers for DMA1. It is 32 bytes aligned because cache coherency operations
     * operate on memory blocks that are 32 byte aligned.
     */
    .dma_mem_d2 (NOLOAD) : {
        . = ALIGN(32);
        _dma_mem_d2_start = .;       /* Symbol at the start of this memory section. */
        *(.dma_mem_d2)
        *(.dma_mem_d2*)
        . = ALIGN(32);
        _dma_mem_d2_end = .;       /* Symbol at the end of this memory section. */
    } > RAM_D2
}