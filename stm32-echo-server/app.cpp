#include <cstdint>
#include <cstring>
#include "app.h"
using namespace std;

// Uncomment this line to use the interrupt based UART RX + DMA transactions that come with the HAL library.
// Otherwise a setup that polls the DMA registers for new data is used, and the DMA and UART errors are interrupt
// based.
//#define INTERRUPT

// These are the HAL handle structures which manage the peripherals in the HAL libraries. We are using a DMA peripheral
// and a UART peripheral. These must be global because they are referenced by the interrupt handler routines.
DMA_HandleTypeDef hdmaUartRx;
UART_HandleTypeDef huartEcho;

// These are the buffers written to by the DMA peripheral. Make sure the buffers are in memory locations accessible
// by the DMA peripheral they are streaming to/from.
static constexpr size_t sRxBufferSize{32};
static __attribute((section(".dmamem1"))) uint8_t sRxBuffer[sRxBufferSize];
static constexpr size_t sTxBufferSize{32};
static __attribute((section(".dmamem1"))) uint8_t sTxBuffer[sTxBufferSize]{"First Data"};

// Static function declarations.
static void startRxStream();
static void dmaErrorHandler(DMA_HandleTypeDef *hdma);

// When initialization fails, this function is called.
// This is a terrible solution for an error handler, but it will do for this example.
void errorHandler() {
    while(1);
}

// This function initializes the peripherals and starts the UART RX stream.
static void init() {
    HAL_Init();
    systemClockConfig();

    ///////////////////////////////////////////////////////////////////////////
    // UART Initialization
    ///////////////////////////////////////////////////////////////////////////

    // Initialization of the UART module. USART3 is connected to the virtual com port of the STLINK chip on
    // NUCLEO-H743ZI2 board. Before you initialize a peripheral, you must turn on its clock. Else nothing happens and
    // you'll be wondering why nothing works.
    __HAL_RCC_USART3_CLK_ENABLE();
    huartEcho.Instance = USART3;                        // Reference to peripheral registers for USART3.
    huartEcho.Init.BaudRate = 115200;                   // UART baud rate.
    huartEcho.Init.WordLength = UART_WORDLENGTH_8B;     // UART byte size.
    huartEcho.Init.StopBits = UART_STOPBITS_1;          // We will use only 1 stop bit.
    huartEcho.Init.Parity = UART_PARITY_NONE;           // We won't use parity.
    huartEcho.Init.Mode = UART_MODE_TX_RX;              // Rx, Tx, or both.
    huartEcho.Init.HwFlowCtl = UART_HWCONTROL_NONE;     // There are no hardware control lines in use.

    // The Rx line is oversampled to account for tolerances in the senders baud rate. Then you can use three samples,
    // or 1 sample in the center of the sampled bits to determine the logic level.
    huartEcho.Init.OverSampling = UART_OVERSAMPLING_16;
    huartEcho.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

    // Option to reduce the peripheral clock frequency for low-powered systems.
    huartEcho.Init.ClockPrescaler = UART_PRESCALER_DIV1;

    // No advanced features are used.
    huartEcho.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    // Configures UART registers.
    if (HAL_UART_Init(&huartEcho) != HAL_OK) {
        errorHandler();
    }

    // To minimize the change of overrun, this UART peripheral will use its internal FIFO. STM32H7 FIFOs are 16words
    // deep. The thresholds indicate how full (for Rx) or how empty (for Tx) the FIFO is before intrerrupts are
    // generated.
    if (HAL_UARTEx_EnableFifoMode(&huartEcho) != HAL_OK) {
        errorHandler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huartEcho, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) {
        errorHandler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huartEcho, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) {
        errorHandler();
    }

    ///////////////////////////////////////////////////////////////////////////
    // GPIO Initialization
    ///////////////////////////////////////////////////////////////////////////

    // Configure GPIO for UART Peripheral
    // PD9     ------> USART3_RX
    // PD8     ------> USART3_TX
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct{0};
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    ///////////////////////////////////////////////////////////////////////////
    // DMA Initialization
    ///////////////////////////////////////////////////////////////////////////

    // A DMA peripheral has multiple streams. DMA1 is used for both the TX and RX stream of USART3.
    // There is also a MDMA and BDMA peripheral in the STM32H7 chip. These cannot connect directly to the
    // UART peripheral we are using. The first thing to do when using a peripheral is to enable it's clock.
    // Always do this before reading/writing to a peripherals registers.
    __HAL_RCC_DMA1_CLK_ENABLE();

    // Stream0 from DMA1 is going to be dedicated to stream RX data from USART3.
    hdmaUartRx.Instance = DMA1_Stream0;

    // Indicates which peripheral DMA is working with, leave it as zero for MEM to MEM stream. This is used to
    // configure DMAMux1.
    hdmaUartRx.Init.Request = DMA_REQUEST_USART3_RX;

    // Direction of the stream: peripheral->memory (eg. UART RX), memory->peripheral (eg. UART TX)
    // or memory-to-memory. Sets the DIR section of the DMA_SxCR register.
    hdmaUartRx.Init.Direction = DMA_PERIPH_TO_MEMORY;

    // Since the UART is read through a single register, we don't want to increment the memory address of this
    // source of data.
    hdmaUartRx.Init.PeriphInc = DMA_PINC_DISABLE;

    // Enable if the memory address is to be incremented as the stream reads/write data from it. This is enabled for
    // both UART RX&TX because we are reading from a buffer. The pointer is incremented by the size of set in the
    // MSIZE sections of the DMA_SxCR register.
    hdmaUartRx.Init.MemInc = DMA_MINC_ENABLE;

    // We are transferring 8bit words from the UART. Don't need to worry about alignment in memory. These options
    // set PSIZE and MSIZE sections of the DMA_SxCR register
    hdmaUartRx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdmaUartRx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;

    // Circular mode is used when continuously streaming data. When the DMA_SxNDTR register (number of data register)
    // becomes zero in circular mode, the previously programmed value is reloaded into it.
    // It is enabled by setting the CIRC section of the DMA_SxCR register.
    hdmaUartRx.Init.Mode = DMA_CIRCULAR;

    // The DMA module can only handle one request to transfer at a time. If two requests are made, the one with
    // higher priority is executed first. If they have the same priority, the one with the lower request number
    // is executed first.
    hdmaUartRx.Init.Priority = DMA_PRIORITY_LOW;

    // Each stream has as 16byte FIFO, into which data is buffered before writing it to the destination.
    // In this example, we explore both the use of the FIFO and without. There is one potential issue with the FIFO.
    // It only writes to the destination once it has hit a certain threshold (4 bytes full). Since we are sending
    // small amounts of data, sometimes the data will remain in the FIFO and we will have to manually flush it out.
    hdmaUartRx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    // The FIFO threshold level is the number of items stored in the FIFO before it is drained to the destination.
    // You need to flush the FIFO to force a transfer of the data in the FIFO when there is data in the FIFO less
    // than the threshold level. For a UART, the FIFO could be flushed when the UART becomes idle.
    hdmaUartRx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;

    // Specifies the number of data items to transfer to/from memory or peripheral in one uninterrupable sequence.
    // Since the word size for the UART is 1byte and the size of each packet can be any size, we will stick
    // with single data item transfers.
    hdmaUartRx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdmaUartRx.Init.PeriphBurst = DMA_PBURST_SINGLE;

    // Steps to initialize DMA.
    // 1. Disable DMA stream, wait until EN section in DMA_SxCR is low before continuing.
    // 2. Set DMA stream configuration register DMA_SxCR.
    // 3. Set DMA stream FIFO control register DMA_SxFCR
    // 4. Clear interrupt flags in either DMA_LIFCR or DMA_HIFCR (these registers contain the flags for all streams)
    // 5. Configure the DMAMUX to connect the peripheral to the stream.
    if (HAL_DMA_Init(&hdmaUartRx) != HAL_OK) {
        errorHandler();
    }

    // This provides a reference to the DMA handle to the UART handle and vice-versa.
    // This is used in UART HAL library in the DMA based functions.
    __HAL_LINKDMA(&huartEcho, hdmarx, hdmaUartRx);

    // Interrupt handlers for UART with DMA
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_SetPriority(USART3_IRQn, 4, 1);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    // sRxBufferSize is written into DMA_SxNDTR register. The size of each data item is given by PSIZE in the DMA_SxCR
    // register. Once the DMA is initialized, the DMA functions in the UART library should be enough to
    #ifdef INTERRUPT
    HAL_UART_Receive_DMA(&huartEcho, sRxBuffer, sRxBufferSize);
    #else
    startRxStream();
    #endif
}

// This starts the DMA for the UART RX stream with error interrupts enabled. DMA transfer are polled in the main
// loop. Use the normal HAL about functions to terminate UART transactions.
//
// An alternative to this might be to still call HAL_UART_Receive_DMA(), then right after disable the two 
// RX complete interrupts. Interrupt enables can be cleared while the DMA is enabled.
static void startRxStream() {
    if (huartEcho.RxState != HAL_UART_STATE_READY || hdmaUartRx.State != HAL_DMA_STATE_READY) {
        return;
    }
    // I may not need to set these parameters.
    huartEcho.pRxBuffPtr = sRxBuffer;
    huartEcho.RxXferSize = sRxBufferSize;
    huartEcho.ErrorCode = HAL_UART_ERROR_NONE;
    huartEcho.RxState = HAL_UART_STATE_BUSY_RX;
    huartEcho.hdmarx->XferErrorCallback = dmaErrorHandler;

    // We make the abort callback null because we use the UART abort calls instead.
    huartEcho.hdmarx->XferAbortCallback = NULL;

    // Enable DMA and UART Error Interrupts
    // ------------------------------------
    // DMA Direct mode error is not enabled because it only happens when MINC is cleared, which is not our case.
    // DMA FIFO error is enabled because although we are not using the FIFO, this error can occur if the peripheral
    //      cannot transfer a data item after several requests. In this case, try increasing the priority of the
    //      request.
    // DMA Transfer error is enabled just in case someone forgets to place the DMA buffer in the correct location.
    //
    // TODO: HAL_DMA_Start_IT() also sets some interrupts associated with the DMAMux. Have a look at these.
    __HAL_DMA_DISABLE(&hdmaUartRx);
    __HAL_DMA_ENABLE_IT(&hdmaUartRx, DMA_IT_TE);
    __HAL_DMA_ENABLE_IT(&hdmaUartRx, DMA_IT_FE);
    __HAL_UART_ENABLE_IT(&huartEcho, UART_IT_ERR);

    // Here we load in the peripheral and memory address, the datasize, and then enable the DMA peripheral.
    HAL_DMA_Start(&hdmaUartRx, reinterpret_cast<uint32_t>(&huartEcho.Instance->RDR),
        reinterpret_cast<uint32_t>(sRxBuffer), sRxBufferSize);

    // Enable the DMA transfer for the receiver request by setting the DMAR bit in the UART CR3 register.
    SET_BIT(huartEcho.Instance->CR3, USART_CR3_DMAR);
}

// We check the value of DMAStream->NDTR register which counts the number of data items in the stream to determine if
// new data has arrived and how much of it has arrived.
static void pollRxStream() {
    size_t localCounter = __HAL_DMA_GET_COUNTER(&hdmaUartRx);
    static size_t dmaHead{0};
    size_t dmaTail = sRxBufferSize - localCounter;
    if (dmaHead < dmaTail) {
        size_t size = dmaTail - dmaHead;
        memcpy(sTxBuffer, sRxBuffer + dmaHead, size);
        HAL_UART_Transmit(&huartEcho, sTxBuffer, size, 1000);
    } else if (dmaHead > dmaTail) {
        size_t size = sRxBufferSize - dmaHead;
        memcpy(sTxBuffer, sRxBuffer + dmaHead, size);
        memcpy(sTxBuffer + size, sRxBuffer, dmaTail);
        HAL_UART_Transmit(&huartEcho, sTxBuffer, size + dmaTail, 1000);
    }
    dmaHead = dmaTail;
}

///////////////////////////////////////////////////////////////////////////////
// Application entry points
///////////////////////////////////////////////////////////////////////////////

// Initialize then if using a polling approach, we check the DMA buffer for new data.
int main() {
    init();
    while (1) {
        HAL_Delay(1000);
        #ifndef INTERRUPT
        pollRxStream();
        #endif
    }
}

#ifdef __cplusplus
extern "C" {
#endif

// In DMA Circular mode, this callback is executed everytime DMA fills up the entire buffer when the transfer complete
// interrupt is enabled.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    pollRxStream();
}

// In DMA Circular mode, this interrupt is executed everytime the first half of the buffer is filled.
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
    pollRxStream();
}

// This is called by both the DMA error interrupts and UART interrupts. This is because
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    switch (huart->hdmarx->ErrorCode) {
    case HAL_DMA_ERROR_NONE:
        break;
    case HAL_DMA_ERROR_TE:
        // DMA peripherals can only read and write to certain memory locations. If you attempt to read and write
        // from/to an unsupported memory address, you will get a transfer error.
        memcpy(sTxBuffer, "RX DMA Transfer Error\n", 22);
        HAL_UART_Transmit(&huartEcho, sTxBuffer, 22, 1000);
        break;
    default:
        memcpy(sTxBuffer, "RX DMA Error\n", 13);
        HAL_UART_Transmit(&huartEcho, sTxBuffer, 13, 1000);
        break;
    }
    if (huart->ErrorCode) {
        memcpy(sTxBuffer, "UART Error\n", 11);
        HAL_UART_Transmit(&huartEcho, sTxBuffer, 11, 1000);
    }
    if (!huart->ErrorCode && !huart->hdmarx->ErrorCode) {
        memcpy(sTxBuffer, "UART Error with no ErrorCode\n", 29);
        HAL_UART_Transmit(&huartEcho, sTxBuffer, 29, 1000);
    }
    // RX is aborted on error. Let's restart.
    #ifdef INTERRUPT
    HAL_UART_Receive_DMA(&huartEcho, sRxBuffer, sRxBufferSize);
    #else
    startRxStream();
    #endif
}

#ifdef __cplusplus
}
#endif

// We need to supply a DMA error callback for a polled circular buffer setup. This turns off the DMA to UART
// transactions, clears the UART RxState and sets the UART error code.
static void dmaErrorHandler(DMA_HandleTypeDef *hdma) {
    CLEAR_BIT(huartEcho.Instance->CR3, USART_CR3_DMAR);
    huartEcho.RxState = HAL_UART_STATE_READY;
    huartEcho.ErrorCode |= HAL_UART_ERROR_DMA;
    HAL_UART_ErrorCallback(&huartEcho);
}