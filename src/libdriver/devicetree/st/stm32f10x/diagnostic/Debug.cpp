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


void TFT_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ´ò¿ªÊ±ÖÓÊ¹ÄÜ */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //PB0 ÍÆÍìÊä³ö ±³¹â
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //ÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//PORTD¸´ÓÃÍÆÍìÊä³ö  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;				 //	//PORTD¸´ÓÃÍÆÍìÊä³ö  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //¸´ÓÃÍÆÍìÊä³ö   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	//PORTE¸´ÓÃÍÆÍìÊä³ö  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;				 //	//PORTD¸´ÓÃÍÆÍìÊä³ö  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //¸´ÓÃÍÆÍìÊä³ö   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//	//PORTG12¸´ÓÃÍÆÍìÊä³ö A0	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_12;	 //	//PORTD¸´ÓÃÍÆÍìÊä³ö  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //¸´ÓÃÍÆÍìÊä³ö   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
}

void TFT_FSMC_Config(void)
{
	/* ³õÊ¼»¯º¯Êý */
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;

	readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //µØÖ·½¨Á¢Ê±¼ä£¨ADDSET£©Îª2¸öHCLK 1/36M=27ns
	readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //µØÖ·±£³ÖÊ±¼ä£¨ADDHLD£©Ä£Ê½AÎ´ÓÃµ½	
	readWriteTiming.FSMC_DataSetupTime = 0x0f;		 // Êý¾Ý±£´æÊ±¼äÎª16¸öHCLK,ÒòÎªÒº¾§Çý¶¯ICµÄ¶ÁÊý¾ÝµÄÊ±ºò£¬ËÙ¶È²»ÄÜÌ«¿ì£¬ÓÈÆä¶Ô1289Õâ¸öIC¡£
	readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	readWriteTiming.FSMC_CLKDivision = 0x00;
	readWriteTiming.FSMC_DataLatency = 0x00;
	readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //Ä£Ê½A 


	writeTiming.FSMC_AddressSetupTime = 0x00;	 //µØÖ·½¨Á¢Ê±¼ä£¨ADDSET£©Îª1¸öHCLK  
	writeTiming.FSMC_AddressHoldTime = 0x00;	 //µØÖ·±£³ÖÊ±¼ä£¨A		
	writeTiming.FSMC_DataSetupTime = 0x03;		 ////Êý¾Ý±£´æÊ±¼äÎª4¸öHCLK	
	writeTiming.FSMC_BusTurnAroundDuration = 0x00;
	writeTiming.FSMC_CLKDivision = 0x00;
	writeTiming.FSMC_DataLatency = 0x00;
	writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //Ä£Ê½A 


	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;//  ÕâÀïÎÒÃÇÊ¹ÓÃNE4 £¬Ò²¾Í¶ÔÓ¦BTCR[6],[7]¡£
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // ²»¸´ÓÃÊý¾ÝµØÖ·
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//´æ´¢Æ÷Êý¾Ý¿í¶ÈÎª16bit   
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  ´æ´¢Æ÷Ð´Ê¹ÄÜ
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // ¶ÁÐ´Ê¹ÓÃ²»Í¬µÄÊ±Ðò
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //¶ÁÐ´Ê±Ðò
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //Ð´Ê±Ðò

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //³õÊ¼»¯FSMCÅäÖÃ

	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);  // Ê¹ÄÜBANK1 
}

typedef volatile struct
{
	u16 TFT_CMD;
	u16 TFT_DATA;
} TFT_TypeDef;
#define TFT_BASE        ((uint32_t)(0x6C000000 | 0x000007FE))
#define TFT             ((TFT_TypeDef *) TFT_BASE)

#define TFT_XMAX 239
#define TFT_YMAX 319

#define WHITE          0xFFFF
#define BLACK          0x0000
#define BLUE           0x001F
#define RED            0xF800
#define MAGENTA        0xF81F
#define GREEN          0x07E0
#define CYAN           0x7FFF
#define YELLOW         0xFFE0


void TFT_WriteCmd(uint16_t cmd)
{
	TFT->TFT_CMD = cmd;
}

void TFT_WriteData(u16 dat)
{
	TFT->TFT_DATA = dat;
}

void TFT_WriteData_Color(u16 color)
{
	TFT->TFT_DATA = color & 0xff00;
	TFT->TFT_DATA = color << 8;
}

uint16_t TFT_ReadData()
{
	return TFT->TFT_DATA;
}

void TFT_Init(void)
{
	uint16_t i;

	TFT_GPIO_Config();
	TFT_FSMC_Config();
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
	Threading::BSPSleepMs(100);
	GPIO_SetBits(GPIOE, GPIO_Pin_1);

	TFT_WriteCmd(0xDB);
	for (volatile size_t i = 0; i < 500; i++) ;
	g_Logger->PutFormat("%p ", &TFT->TFT_DATA);
	g_Logger->PutFormat("%x ", TFT_ReadData());
	g_Logger->PutFormat("%x ", TFT_ReadData());
	g_Logger->PutFormat("%x ", TFT_ReadData());
	g_Logger->PutFormat("%x ", TFT_ReadData());

	return;

	//************* Start Initial Sequence **********//	
	TFT_WriteData(0x39);
	TFT_WriteData(0x2C);
	TFT_WriteData(0x00);
	TFT_WriteData(0x34);
	TFT_WriteData(0x02);

	TFT_WriteCmd(0xCF);
	TFT_WriteData(0x00);
	TFT_WriteData(0xa2); //c1
	TFT_WriteData(0Xf0);  //30

	TFT_WriteCmd(0xE8);
	TFT_WriteData(0x84); 	 //85
	TFT_WriteData(0x11); 	 //00
	TFT_WriteData(0x7a); 	 //79


	TFT_WriteCmd(0xEA);
	TFT_WriteData(0x66);  //00
	TFT_WriteData(0x00);

	TFT_WriteCmd(0xED);
	TFT_WriteData(0x55); 	//64
	TFT_WriteData(0x01); 	//03
	TFT_WriteData(0X23); 	//12
	TFT_WriteData(0X01); 	//81

	TFT_WriteCmd(0xF7);
	TFT_WriteData(0x20); 	//20


	TFT_WriteCmd(0xC0);    //Power control 
	TFT_WriteData(0x1c);   //VRH[5:0] 	//1c

	TFT_WriteCmd(0xC1);    //Power control 
	TFT_WriteData(0x13);   //SAP[2:0];BT[3:0] 	//13

	TFT_WriteCmd(0xC5);    //VCM control 
	TFT_WriteData(0x23);
	TFT_WriteData(0x3F);

	TFT_WriteCmd(0xC7);    //VCM control2 
	TFT_WriteData(0xA5);

	TFT_WriteCmd(0xB1);
	TFT_WriteData(0x00);
	TFT_WriteData(0x17);

	TFT_WriteCmd(0x3A);
	TFT_WriteData(0x55);

	TFT_WriteCmd(0xB6);
	TFT_WriteData(0x0A);
	TFT_WriteData(0xa2);  //a2
	TFT_WriteData(0x27);
	TFT_WriteData(0x00);

	TFT_WriteCmd(0x36);    // Memory Access Control 
	TFT_WriteData(0x08); 	  //08	BGR

	TFT_WriteCmd(0xF2);    // 3Gamma Function Disable 
	TFT_WriteData(0x02); 	//00

	TFT_WriteCmd(0x26);    //Gamma curve selected 
	TFT_WriteData(0x01);

	TFT_WriteCmd(0xE0);    //Set Gamma 
	TFT_WriteData(0x0F);
	TFT_WriteData(0x14);
	TFT_WriteData(0x13);
	TFT_WriteData(0x0C);
	TFT_WriteData(0x0E);
	TFT_WriteData(0x05);
	TFT_WriteData(0x45);
	TFT_WriteData(0x85);
	TFT_WriteData(0x36);
	TFT_WriteData(0x09);
	TFT_WriteData(0x14);
	TFT_WriteData(0x05);
	TFT_WriteData(0x09);
	TFT_WriteData(0x03);
	TFT_WriteData(0x00);

	TFT_WriteCmd(0XE1);    //Set Gamma 
	TFT_WriteData(0x00);
	TFT_WriteData(0x24);
	TFT_WriteData(0x26);
	TFT_WriteData(0x03);
	TFT_WriteData(0x0F);
	TFT_WriteData(0x04);
	TFT_WriteData(0x3F);
	TFT_WriteData(0x14);
	TFT_WriteData(0x52);
	TFT_WriteData(0x04);
	TFT_WriteData(0x10);
	TFT_WriteData(0x0E);
	TFT_WriteData(0x38);
	TFT_WriteData(0x39);
	TFT_WriteData(0x0F);

	TFT_WriteCmd(0x2A);
	TFT_WriteData(0x00);
	TFT_WriteData(0x00);
	TFT_WriteData(0x00);
	TFT_WriteData(0xEF);

	TFT_WriteCmd(0x2B);
	TFT_WriteData(0x00);
	TFT_WriteData(0x00);
	TFT_WriteData(0x01);
	TFT_WriteData(0x3F);

	TFT_WriteCmd(0x11);    //Exit Sleep 
	for (i = 50000; i>0; i--);
	for (i = 50000; i>0; i--);
	for (i = 50000; i>0; i--);
	for (i = 50000; i>0; i--);
	for (i = 50000; i>0; i--);
	for (i = 50000; i>0; i--);
	TFT_WriteCmd(0x29);    //Display on
}

void TFT_SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
	TFT_WriteCmd(0x2A);

	TFT_WriteData(xStart / 256);
	TFT_WriteData(xStart % 256);
	TFT_WriteData(xEnd / 256);
	TFT_WriteData(xEnd % 256);

	TFT_WriteCmd(0x2b);
	TFT_WriteData(yStart / 256);
	TFT_WriteData(yStart % 256);
	TFT_WriteData(yEnd / 256);
	TFT_WriteData(yEnd % 256);
	TFT_WriteCmd(0x2c);
}

void TFT_ClearScreen(uint16_t color)
{
	uint16_t i, j;

	TFT_SetWindow(0, 0, TFT_XMAX, TFT_YMAX);
	for (i = 0; i<TFT_XMAX + 1; i++)
	{
		for (j = 0; j<TFT_YMAX + 1; j++)
		{
			TFT_WriteData_Color(color);
		}
	}
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

	TFT_Init();
	while (1);
	//TFT_SetWindow(0, 0, TFT_XMAX, TFT_YMAX);
	//TFT_WriteData_Color(BLUE);
	//TFT_ClearScreen(BLUE);
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
