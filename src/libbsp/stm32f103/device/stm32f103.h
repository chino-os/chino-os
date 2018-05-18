#pragma once

#ifdef __cplusplus
extern "C" {
#endif 

#ifdef __INTELLISENSE__
#define STM32F10X_HD
#define _RTE_
#define __UVISION_VERSION="523"
#endif
#include <stdint.h>
#include <stddef.h>
#include "stm32f10x.h"
#include "rcc/stm32f10x_rcc.h"
#include "gpio/stm32f10x_gpio.h"
#include "usart/stm32f10x_usart.h"
#include "system_stm32f10x.h"

#ifdef __cplusplus
}
#endif 