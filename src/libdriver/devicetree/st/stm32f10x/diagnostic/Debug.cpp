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
#include <stm32f10x_spi.h>
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

int uart_putc(int ch)
{
	USART1->SR;
	USART_SendData(USART1, (unsigned char)ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);

	return(ch);
}

void SPI2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	/* SPI��IO�ں�SPI�����ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* SPI��IO������ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/***************************************************************************/
	/************************* ����SPI�Ĳ��� ***********************************/
	/***************************************************************************/

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//ѡ��ȫ˫��SPIģʽ
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;     //����ģʽ
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //8λSPI
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;       //ʱ�����ոߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;      //�ڵڶ���ʱ�Ӳɼ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		  //Nssʹ���������
													  /* ѡ������Ԥ��ƵΪ256 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//�����λ��ʼ����
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Cmd(SPI2, ENABLE);
	SPI_Init(SPI2, &SPI_InitStructure);
}

uint8_t SPI2_WriteReadData(uint8_t dat)
{
	uint16_t i = 0;

	/* �����ͻ������� */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
	{
		i++;
		if (i > 10000)
		{
			return 0xFF;
		}
	}

	/* �������� */
	SPI_I2S_SendData(SPI2, dat);

	/* �ȴ����ջ�����Ϊ�ǿ� */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	/* ����ȡ������ֵ���� */
	return SPI_I2S_ReceiveData(SPI2);
}

void SPI2_SetSpeed(uint8_t Speed)
{

	SPI2->CR1 &= 0xFFC7;
	SPI2->CR1 |= Speed;
	SPI_Cmd(SPI2, ENABLE);

}

#define FLASH_CS_SET GPIO_SetBits(GPIOG, GPIO_Pin_13)
#define FLASH_CS_CLR {GPIO_ResetBits(GPIOG, GPIO_Pin_13);\
GPIO_SetBits(GPIOG, GPIO_Pin_14);GPIO_SetBits(GPIOB, GPIO_Pin_12);}

uint16_t FLASH_ReadID(void)
{
	uint16_t id = 0;

	FLASH_CS_CLR;            //��Ƭѡ

	SPI2_WriteReadData(0x90);//���Ͷ�ȡID����	    
	SPI2_WriteReadData(0x00);
	SPI2_WriteReadData(0x00);
	SPI2_WriteReadData(0x00);

	id |= SPI2_WriteReadData(0xFF) << 8; //��ȡ16λID 
	id |= SPI2_WriteReadData(0xFF);

	FLASH_CS_SET;			 //�ر�Ƭѡ   
	return id;
}

uint16_t FLASH_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOG, ENABLE);

	/* SD_CS PG14/ FLASH_CS PG13 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_SetBits(GPIOG, GPIO_Pin_14 | GPIO_Pin_13);

	/* ENC28J60_CS PB12 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);

	/* ��ʼ��SPI2 */
	SPI2_Config();

	/* ��ʼ��FLASHҪ��ȡоƬIDһ�� */
	return(FLASH_ReadID());
}

void Chino::Diagnostic::BSPInitializeDebug(const BootParameters& bootParams)
{
	RCC_Configuration();
	GPIO_Configuration();
	USART_Configuration(115200);

	//auto id = FLASH_Init();
	//g_Logger->PutFormat("ID: %x\n", id);
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
