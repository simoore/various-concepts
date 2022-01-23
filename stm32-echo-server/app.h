#ifndef APP_H
#define APP_H

// There must be a stm32h7xx_hal_conf.h header defined an accessible to the HAL library for configuring various 
// features in the HAL library for this project.
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_it.h"
#include "base.h"

void errorHandler();

extern DMA_HandleTypeDef hdmaUartRx;
extern DMA_HandleTypeDef hdmaUartTx;
extern UART_HandleTypeDef huartEcho;

#endif // APP_H