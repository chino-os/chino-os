//
// Kernel Diagnostic
//
#include <kernel/utils.hpp>
#include <libbsp/bsp.hpp>
#include "../device/stm32f103.h"

using namespace Chino;

void RCC_Configuration(void)
{
	//---------????????--------------------  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //??APB2???GPIOA???   
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USARTx_Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USARTx_Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}

void USART_Configuration(u32 BaudRate)
{
	//1??????????USART????  
	USART_InitTypeDef USART_InitStructure;

	//2?????????,?? ???????????????????????????????USART??(?????)  
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	//3?????USART_Init();?????????????USART1?????  
	USART_Init(USART1, &USART_InitStructure);

	//4?????USART_Cmd();?USART1????  
	USART_Cmd(USART1, ENABLE);

}

int uart_putc(int ch)
{
	USART1->SR;  //USART_GetFlagStatus(USART1, USART_FLAG_TC) ??????????????
				 //????????
	USART_SendData(USART1, (unsigned char)ch);
	//??????
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);

	return(ch);
}

void Chino::Diagnostic::BSPInitializeDebug(const BootParameters& bootParams)
{
	RCC_Configuration();
	GPIO_Configuration();
	USART_Configuration(115200);
}

void Chino::Diagnostic::BSPDebugPutChar(wchar_t chr)
{
	uart_putc((uint8_t)chr);
}

void Chino::Diagnostic::BSPDebugBlueScreen()
{
}

void Chino::Diagnostic::BSPDebugClearScreen()
{
}
