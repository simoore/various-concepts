#include "app.h"

///////////////////////////////////////////////////////////////////////////////
// Cortex Processor Interruption and Exception Handlers        
///////////////////////////////////////////////////////////////////////////////

// This function handles Non maskable interrupt.
void NMI_Handler(void) {}

// This function handles Hard fault interrupt.
void HardFault_Handler(void) {
    while (1);
}

// This function handles Memory management fault.
void MemManage_Handler(void) {
    while (1);
}

// This function handles Pre-fetch fault, memory access fault.
void BusFault_Handler(void) {
    while (1);
}

// This function handles Undefined instruction or illegal state.
void UsageFault_Handler(void) {
    while (1);
}

// This function handles System service call via SWI instruction.
void SVC_Handler(void) {}

// This function handles Debug monitor.
void DebugMon_Handler(void) {}

// This function handles Pendable request for system service.
void PendSV_Handler(void) {}

// This function handles System tick timer.
void SysTick_Handler(void) {
    HAL_IncTick();
}

///////////////////////////////////////////////////////////////////////////////
// STM32H7xx Peripheral Interrupt Handlers                                    
///////////////////////////////////////////////////////////////////////////////

void DMA1_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdmaUartRx);
}

void USART3_IRQHandler(void) {
    HAL_UART_IRQHandler(&huartEcho);
}
