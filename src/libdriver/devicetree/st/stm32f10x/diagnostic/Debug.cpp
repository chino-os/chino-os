//
// Kernel Diagnostic
//
#include <kernel/utils.hpp>
#include <libbsp/bsp.hpp>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_fsmc.h>

using namespace Chino;

void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void USART_Configuration(u32 BaudRate)
{
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);

}

void I2C1_Init()
{
	I2C_InitTypeDef I2C_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x10;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;
	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
}
u8 I2C_Read(u8 nAddr)
{
	I2C_AcknowledgeConfig(I2C1, ENABLE); //使能应答
	I2C_GenerateSTART(I2C1, ENABLE); //发送一个开始位
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) { ; } //等待EV5
	I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter); //发送一个伪写指令
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) { ; }//等待EV6
	I2C_SendData(I2C1, nAddr);//发送读地址
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) { ; } //等待EV8

	I2C_GenerateSTART(I2C1, ENABLE); //发送一个开始位
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) { ; } //等待EV5
	I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Receiver); //发送一个读指令
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) { ; } //等待EV6
	I2C_AcknowledgeConfig(I2C1, DISABLE); //应答使能关闭
	I2C_GenerateSTOP(I2C1, ENABLE); //发送一个停止位
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) { ; } //等待EV7
	return I2C_ReceiveData(I2C1); //返回读到的数据
}
static int8_t I2C_Write(uint8_t writeAddr, uint8_t value)
{
	/* 检测I2C总线是否忙碌 */
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	/* 发送起始信号 */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* 检测EV5，即是否启动为主机模式 */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* 发送写器件地址 */
	I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter);//发送模式

	/* 检测EV6并清除，*/
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* 发送写地址 */
	I2C_SendData(I2C1, writeAddr);

	/* 检测EV8 */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	/* 发送数据 */
	I2C_SendData(I2C1, value);
	/* 检测EV8 */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_GenerateSTOP(I2C1, ENABLE);

	return 0;
}

int uart_putc(int ch)
{
	USART1->SR;
	USART_SendData(USART1, (unsigned char)ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);

	return(ch);
}

void Chino::Diagnostic::BSPInitializeDebug(const BootParameters& bootParams)
{
	RCC_Configuration();
	GPIO_Configuration();
	USART_Configuration(115200);

	//TFT_Init();
	//LCD_Clear(BLUE);
	//I2C1_Init();
	//BSPDebugPutChar('1'); I2C_Write(0x10, 1);
	//Threading::BSPSleepMs(10);
	//BSPDebugPutChar('2'); I2C_Write(0x10, 1);
	//Threading::BSPSleepMs(10);
	//BSPDebugPutChar('3'); I2C_Read(0x10);
	//BSPDebugPutChar('4');
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
