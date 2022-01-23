# STM32 Echo Server of UART using DMA in Circular Mode

## STM32 Debugging in Visual Studio Code

The Cortex-Debug plugin adds debugging capabilities to ARM Cortex-M devices in VSCode.
[Cortex-Debug Wiki](https://github.com/Marus/cortex-debug/wiki)

One thing that I found was missing was the disply of variable addresses when execution was paused. You can query
the address of the variable in the GDB terminal. For example to find the memory address of `sRxBUffer` use:
```
p &sRxBuffer
```
or 
```
info address sRxBuffer
```

## Some Design Choices

* The FIFO in the DMA peripheral was avoided because it all drains to the desttination once it has a minimum of 4
    bytes in the FIFO. If you are dealing with small quantities of data then flushing the DMA to get any remainging 
    data out of the queue seems like uncessary work. 

## References

* [STM32 DMA Cheat Sheet](https://adammunich.com/stm32-dma-cheat-sheet/)
* "STM32H742, STM32H743/753 and STM32H750 Value line advanced Arm®-based 32-bit MCUs", 
    April 2019 RM0433 Rev 6, www.st.com

## Examples of Using DMA on STM32

http://www.lucadavidian.com/2017/11/17/stm32f4-using-the-dma-controller-with-adc/
This is an example of using DMA in STM32F4 with an ADC peripheral.

[MDMA software expansion for STM32Cube](https://www.st.com/en/embedded-software/x-cube-mdma.html)
