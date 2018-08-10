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
#include <stm32f10x_tim.h>

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

static void delay_ms(int ms)
{
	Threading::BSPSleepMs(ms * 6);
}

#include <stdio.h>
#include "enc28j60.h"	  


static u8 ENC28J60BANK;
static u32 NextPacketPtr;
u32 uip_timer = 0;//uip ��ʱ����ÿ10ms����1.

				  //��ʱ��6�жϷ������	 
void TIM6_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		uip_timer++;//uip��ʱ������1	
	}
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 

}

//������ʱ��6�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
void TIM6_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM6, TIM_IT_Update | TIM_IT_Trigger, ENABLE);//ʹ�ܶ�ʱ��6���´����ж�

	TIM_Cmd(TIM6, ENABLE);  //ʹ��TIMx����

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 								 
}

u8 SPI2_ReadWriteByte(u8 dat)		 //SPI2读写一个字节
{
	u8 t;
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)	//等待发送缓存器空
	{
		t++;
		if (t >= 200)return 0;	//超时返回错误标志	
	}
	SPI_I2S_SendData(SPI2, dat); //发送数据
	t = 0;
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)	//等待接收
	{
		t++;
		if (t >= 200)return 0;	//超时返回错误标志	
	}
	return SPI_I2S_ReceiveData(SPI2); //返回最近SPI2接收的数据			
}
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)	//设置SPI2的速度
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1 &= 0XFFC7;		 //修改BR[2:0]值
	SPI2->CR1 |= SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPI2, ENABLE);
}

//复位ENC28J60
//包括SPI初始化/IO初始化等

static void ENC28J60_SPI2_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//SPI2时钟使能 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOG, ENABLE);//PORTB,D,G时钟使能 

																									   //这里PG13和PG14拉高,是为了防止FLASH和SD影响.
																									   //因为他们共用一个SPI口.  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz				 
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_SetBits(GPIOG, GPIO_Pin_13 | GPIO_Pin_14);

	//NRF24L01_CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_SetBits(GPIOF, GPIO_Pin_9);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_12;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_12);

	//SPI2 		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB
	GPIO_SetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);  //PB13/14/15上拉


	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第一个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	SPI_Cmd(SPI2, ENABLE); //使能SPI外设

	SPI2_ReadWriteByte(0xff);//启动传输	 

}


void ENC28J60_Reset(void)
{

	ENC28J60_SPI2_Init();//SPI2初始化
	SPI2_SetSpeed(SPI_BaudRatePrescaler_4);	//SPI2 SCK频率为36M/4=9Mhz
	//TIM6_Int_Init(1000, 719);//100Khz计数频率，计数到1000为10ms
	ENC28J60_RST = 0;			//复位ENC28J60
	delay_ms(10);
	ENC28J60_RST = 1;			//复位结束				    
	delay_ms(10);
}
//读取ENC28J60寄存器(带操作码) 
//op：操作码
//addr:寄存器地址/参数
//返回值:读到的数据
u8 ENC28J60_Read_Op(u8 op, u8 addr)
{
	u8 dat = 0;
	ENC28J60_CS = 0;
	dat = op | (addr&ADDR_MASK);
	SPI2_ReadWriteByte(dat);
	dat = SPI2_ReadWriteByte(0xFF);
	//如果是读取MAC/MII寄存器,则第二次读到的数据才是正确的,见手册29页
	if (addr & 0x80)dat = SPI2_ReadWriteByte(0xFF);
	ENC28J60_CS = 1;
	return dat;
}
//读取ENC28J60寄存器(带操作码) 
//op：操作码
//addr:寄存器地址
//data:参数
void ENC28J60_Write_Op(u8 op, u8 addr, u8 data)
{
	u8 dat = 0;
	ENC28J60_CS = 0;
	dat = op | (addr&ADDR_MASK);
	SPI2_ReadWriteByte(dat);
	SPI2_ReadWriteByte(data);
	ENC28J60_CS = 1;
}
//读取ENC28J60接收缓存数据
//len:要读取的数据长度
//data:输出数据缓存区(末尾自动添加结束符)
void ENC28J60_Read_Buf(u32 len, u8* data)
{
	ENC28J60_CS = 0;
	SPI2_ReadWriteByte(ENC28J60_READ_BUF_MEM);
	while (len)
	{
		len--;
		*data = (u8)SPI2_ReadWriteByte(0);
		data++;
	}
	*data = '\0';
	ENC28J60_CS = 1;
}
//向ENC28J60写发送缓存数据
//len:要写入的数据长度
//data:数据缓存区 
void ENC28J60_Write_Buf(u32 len, u8* data)
{
	ENC28J60_CS = 0;
	SPI2_ReadWriteByte(ENC28J60_WRITE_BUF_MEM);
	while (len)
	{
		len--;
		SPI2_ReadWriteByte(*data);
		data++;
	}
	ENC28J60_CS = 1;
}
//设置ENC28J60寄存器Bank
//ban:要设置的bank
void ENC28J60_Set_Bank(u8 bank)
{
	if ((bank&BANK_MASK) != ENC28J60BANK)//和当前bank不一致的时候,才设置
	{
		ENC28J60_Write_Op(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
		ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET, ECON1, (bank&BANK_MASK) >> 5);
		ENC28J60BANK = (bank&BANK_MASK);
	}
}
//读取ENC28J60指定寄存器 
//addr:寄存器地址
//返回值:读到的数据
u8 ENC28J60_Read(u8 addr)
{
	ENC28J60_Set_Bank(addr);//设置BANK		 
	return ENC28J60_Read_Op(ENC28J60_READ_CTRL_REG, addr);
}
//向ENC28J60指定寄存器写数据
//addr:寄存器地址
//data:要写入的数据		 
void ENC28J60_Write(u8 addr, u8 data)
{
	ENC28J60_Set_Bank(addr);
	ENC28J60_Write_Op(ENC28J60_WRITE_CTRL_REG, addr, data);
}
//向ENC28J60的PHY寄存器写入数据
//addr:寄存器地址
//data:要写入的数据		 
void ENC28J60_PHY_Write(u8 addr, u32 data)
{
	u16 retry = 0;
	ENC28J60_Write(MIREGADR, addr);	//设置PHY寄存器地址
	ENC28J60_Write(MIWRL, data);		//写入数据
	ENC28J60_Write(MIWRH, data >> 8);
	while ((ENC28J60_Read(MISTAT)&MISTAT_BUSY) && retry<0XFFF)retry++;//等待写入PHY结束		  
}
//初始化ENC28J60
//macaddr:MAC地址
//返回值:0,初始化成功;
//       1,初始化失败;
u8 ENC28J60_Init(u8* macaddr)
{
	u16 retry = 0;
	ENC28J60_Reset();
	ENC28J60_Write_Op(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);//软件复位
	while (!(ENC28J60_Read(ESTAT)&ESTAT_CLKRDY) && retry<500)//等待时钟稳定
	{
		retry++;
		delay_ms(1);
	};
	if (retry >= 500)return 1;//ENC28J60初始化失败
							  // do bank 0 stuff
							  // initialize receive buffer
							  // 16-bit transfers,must write low byte first
							  // set receive buffer start address	   设置接收缓冲区地址  8K字节容量
	NextPacketPtr = RXSTART_INIT;
	// Rx start
	//接收缓冲器由一个硬件管理的循环FIFO 缓冲器构成。
	//寄存器对ERXSTH:ERXSTL 和ERXNDH:ERXNDL 作
	//为指针，定义缓冲器的容量和其在存储器中的位置。
	//ERXST和ERXND指向的字节均包含在FIFO缓冲器内。
	//当从以太网接口接收数据字节时，这些字节被顺序写入
	//接收缓冲器。 但是当写入由ERXND 指向的存储单元
	//后，硬件会自动将接收的下一字节写入由ERXST 指向
	//的存储单元。 因此接收硬件将不会写入FIFO 以外的单
	//元。
	//设置接收起始字节
	ENC28J60_Write(ERXSTL, RXSTART_INIT & 0xFF);
	ENC28J60_Write(ERXSTH, RXSTART_INIT >> 8);
	//ERXWRPTH:ERXWRPTL 寄存器定义硬件向FIFO 中
	//的哪个位置写入其接收到的字节。 指针是只读的，在成
	//功接收到一个数据包后，硬件会自动更新指针。 指针可
	//用于判断FIFO 内剩余空间的大小  8K-1500。 
	//设置接收读指针字节
	ENC28J60_Write(ERXRDPTL, RXSTART_INIT & 0xFF);
	ENC28J60_Write(ERXRDPTH, RXSTART_INIT >> 8);
	//设置接收结束字节
	ENC28J60_Write(ERXNDL, RXSTOP_INIT & 0xFF);
	ENC28J60_Write(ERXNDH, RXSTOP_INIT >> 8);
	//设置发送起始字节
	ENC28J60_Write(ETXSTL, TXSTART_INIT & 0xFF);
	ENC28J60_Write(ETXSTH, TXSTART_INIT >> 8);
	//设置发送结束字节
	ENC28J60_Write(ETXNDL, TXSTOP_INIT & 0xFF);
	ENC28J60_Write(ETXNDH, TXSTOP_INIT >> 8);
	// do bank 1 stuff,packet filter:
	// For broadcast packets we allow only ARP packtets
	// All other packets should be unicast only for our mac (MAADR)
	//
	// The pattern to match on is therefore
	// Type     ETH.DST
	// ARP      BROADCAST
	// 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
	// in binary these poitions are:11 0000 0011 1111
	// This is hex 303F->EPMM0=0x3f,EPMM1=0x30
	//接收过滤器
	//UCEN：单播过滤器使能位
	//当ANDOR = 1 时：
	//1 = 目标地址与本地MAC 地址不匹配的数据包将被丢弃
	//0 = 禁止过滤器
	//当ANDOR = 0 时：
	//1 = 目标地址与本地MAC 地址匹配的数据包会被接受
	//0 = 禁止过滤器
	//CRCEN：后过滤器CRC 校验使能位
	//1 = 所有CRC 无效的数据包都将被丢弃
	//0 = 不考虑CRC 是否有效
	//PMEN：格式匹配过滤器使能位
	//当ANDOR = 1 时：
	//1 = 数据包必须符合格式匹配条件，否则将被丢弃
	//0 = 禁止过滤器
	//当ANDOR = 0 时：
	//1 = 符合格式匹配条件的数据包将被接受
	//0 = 禁止过滤器
	ENC28J60_Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
	ENC28J60_Write(EPMM0, 0x3f);
	ENC28J60_Write(EPMM1, 0x30);
	ENC28J60_Write(EPMCSL, 0xf9);
	ENC28J60_Write(EPMCSH, 0xf7);
	// do bank 2 stuff
	// enable MAC receive
	//bit 0 MARXEN：MAC 接收使能位
	//1 = 允许MAC 接收数据包
	//0 = 禁止数据包接收
	//bit 3 TXPAUS：暂停控制帧发送使能位
	//1 = 允许MAC 发送暂停控制帧（用于全双工模式下的流量控制）
	//0 = 禁止暂停帧发送
	//bit 2 RXPAUS：暂停控制帧接收使能位
	//1 = 当接收到暂停控制帧时，禁止发送（正常操作）
	//0 = 忽略接收到的暂停控制帧
	ENC28J60_Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
	// bring MAC out of reset
	//将MACON2 中的MARST 位清零，使MAC 退出复位状态。
	ENC28J60_Write(MACON2, 0x00);
	// enable automatic padding to 60bytes and CRC operations
	//bit 7-5 PADCFG2:PACDFG0：自动填充和CRC 配置位
	//111 = 用0 填充所有短帧至64 字节长，并追加一个有效的CRC
	//110 = 不自动填充短帧
	//101 = MAC 自动检测具有8100h 类型字段的VLAN 协议帧，并自动填充到64 字节长。如果不
	//是VLAN 帧，则填充至60 字节长。填充后还要追加一个有效的CRC
	//100 = 不自动填充短帧
	//011 = 用0 填充所有短帧至64 字节长，并追加一个有效的CRC
	//010 = 不自动填充短帧
	//001 = 用0 填充所有短帧至60 字节长，并追加一个有效的CRC
	//000 = 不自动填充短帧
	//bit 4 TXCRCEN：发送CRC 使能位
	//1 = 不管PADCFG如何，MAC都会在发送帧的末尾追加一个有效的CRC。 如果PADCFG规定要
	//追加有效的CRC，则必须将TXCRCEN 置1。
	//0 = MAC不会追加CRC。 检查最后4 个字节，如果不是有效的CRC 则报告给发送状态向量。
	//bit 0 FULDPX：MAC 全双工使能位
	//1 = MAC工作在全双工模式下。 PHCON1.PDPXMD 位必须置1。
	//0 = MAC工作在半双工模式下。 PHCON1.PDPXMD 位必须清零。
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX);
	// set inter-frame gap (non-back-to-back)
	//配置非背对背包间间隔寄存器的低字节
	//MAIPGL。 大多数应用使用12h 编程该寄存器。
	//如果使用半双工模式，应编程非背对背包间间隔
	//寄存器的高字节MAIPGH。 大多数应用使用0Ch
	//编程该寄存器。
	ENC28J60_Write(MAIPGL, 0x12);
	ENC28J60_Write(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	//配置背对背包间间隔寄存器MABBIPG。当使用
	//全双工模式时，大多数应用使用15h 编程该寄存
	//器，而使用半双工模式时则使用12h 进行编程。
	ENC28J60_Write(MABBIPG, 0x15);
	// Set the maximum packet size which the controller will accept
	// Do not send packets longer than MAX_FRAMELEN:
	// 最大帧长度  1500
	ENC28J60_Write(MAMXFLL, MAX_FRAMELEN & 0xFF);
	ENC28J60_Write(MAMXFLH, MAX_FRAMELEN >> 8);
	// do bank 3 stuff
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	//设置MAC地址
	ENC28J60_Write(MAADR5, macaddr[0]);
	ENC28J60_Write(MAADR4, macaddr[1]);
	ENC28J60_Write(MAADR3, macaddr[2]);
	ENC28J60_Write(MAADR2, macaddr[3]);
	ENC28J60_Write(MAADR1, macaddr[4]);
	ENC28J60_Write(MAADR0, macaddr[5]);
	//配置PHY为全双工  LEDB为拉电流
	ENC28J60_PHY_Write(PHCON1, PHCON1_PDPXMD);
	// no loopback of transmitted frames	 禁止环回
	//HDLDIS：PHY 半双工环回禁止位
	//当PHCON1.PDPXMD = 1 或PHCON1.PLOOPBK = 1 时：
	//此位可被忽略。
	//当PHCON1.PDPXMD = 0 且PHCON1.PLOOPBK = 0 时：
	//1 = 要发送的数据仅通过双绞线接口发出
	//0 = 要发送的数据会环回到MAC 并通过双绞线接口发出
	ENC28J60_PHY_Write(PHCON2, PHCON2_HDLDIS);
	// switch to bank 0
	//ECON1 寄存器
	//寄存器3-1 所示为ECON1 寄存器，它用于控制
	//ENC28J60 的主要功能。 ECON1 中包含接收使能、发
	//送请求、DMA 控制和存储区选择位。	   
	ENC28J60_Set_Bank(ECON1);
	// enable interrutps
	//EIE： 以太网中断允许寄存器
	//bit 7 INTIE： 全局INT 中断允许位
	//1 = 允许中断事件驱动INT 引脚
	//0 = 禁止所有INT 引脚的活动（引脚始终被驱动为高电平）
	//bit 6 PKTIE： 接收数据包待处理中断允许位
	//1 = 允许接收数据包待处理中断
	//0 = 禁止接收数据包待处理中断
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
	// enable packet reception
	//bit 2 RXEN：接收使能位
	//1 = 通过当前过滤器的数据包将被写入接收缓冲器
	//0 = 忽略所有接收的数据包
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
	if (ENC28J60_Read(MAADR5) == macaddr[0])return 0;//初始化成功
	else return 1;

}
//读取EREVID
u8 ENC28J60_Get_EREVID(void)
{
	//在EREVID 内也存储了版本信息。 EREVID 是一个只读控
	//制寄存器，包含一个5 位标识符，用来标识器件特定硅片
	//的版本号
	return ENC28J60_Read(EREVID);
}

//通过ENC28J60发送数据包到网络
//len:数据包大小
//packet:数据包
void ENC28J60_Packet_Send(u32 len, u8* packet)
{
	//设置发送缓冲区地址写指针入口
	ENC28J60_Write(EWRPTL, TXSTART_INIT & 0xFF);
	ENC28J60_Write(EWRPTH, TXSTART_INIT >> 8);
	//设置TXND指针，以对应给定的数据包大小	   
	ENC28J60_Write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
	ENC28J60_Write(ETXNDH, (TXSTART_INIT + len) >> 8);
	//写每包控制字节（0x00表示使用macon3的设置） 
	ENC28J60_Write_Op(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	//复制数据包到发送缓冲区
	//printf("len:%d\r\n",len);	//监视发送数据长度
	ENC28J60_Write_Buf(len, packet);
	//发送数据到网络
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
	//复位发送逻辑的问题。参见Rev. B4 Silicon Errata point 12.
	if ((ENC28J60_Read(EIR)&EIR_TXERIF))ENC28J60_Write_Op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
}

/*
********************************************************************************
* 函 数 名: enc28j60_init_send
* 功能说明: 数据包发送函数【1】 修改发送缓冲区，并发送控制字节
*           控制字节为0，采用macon3参数，自动填充0，使能CRC校验
* 参    数: int len       数据包长度
* 返 回 值: 无
* 使用说明: 适用于LwIP，在low_level_out中使用，替代initiate transfer();
* 调用方法: enc28j60_init_send(p->tot_len); // 表示pbuf数据包整体长度
********************************************************************************
*/
void enc28j60_init_send(int len)
{
	/* 查询发送逻辑复位位 */
	while ((ENC28J60_Read(ECON1) & ECON1_TXRTS) != 0);

	/* 设置发送缓冲区起始地址 */
	ENC28J60_Write(EWRPTL, TXSTART_INIT & 0xFF);
	ENC28J60_Write(EWRPTH, TXSTART_INIT >> 8);

	/* 设置发送缓冲区结束地址 该值对应发送数据包长度 */
	ENC28J60_Write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
	ENC28J60_Write(ETXNDH, (TXSTART_INIT + len) >> 8);

	/* 发送控制字节 控制字节为0x00,表示使用macon3设置 */
	ENC28J60_Write_Op(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
}

/*
********************************************************************************
* 函 数 名: enc28j60_start_send
* 功能说明: 数据包发送函数【3】 启动发送
* 参    数: 无
* 返 回 值: 无
* 使用说明: 适用于LwIP，在low_level_out中使用
*           替代signal that packet should be sent();
* 调用方法: enc28j60_start_send
********************************************************************************
*/
void enc28j60_start_send(void)
{
	/* 启动发送 */
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

	/* 复位发送逻辑的问题。参见 Rev. B4 Silicon Errata point 12. */
	if ((ENC28J60_Read(EIR) & EIR_TXERIF))
	{
		ENC28J60_Set_Bank(ECON1);
		ENC28J60_Write_Op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
	}
}

/*
********************************************************************************
* 函 数 名: enc28j60_packet_getcount
* 功能说明: 获得enc28j60数据包个数
* 参    数: 无
* 返 回 值: 数据包个数
* 使用说明: 适用于uIP或者LwIP，可定时循环调用该数据
*           若LwIP系统，可调用ethernetif_input();
* 调用方法:
********************************************************************************
*/
int enc28j60_packet_getcount(void)
{
	return (int)ENC28J60_Read(EPKTCNT);
}
/*
********************************************************************************
* 函 数 名: enc28j60_packet_getlen
* 功能说明: 获得enc28j60数据包长度
* 参    数: 无
* 返 回 值: 数据包长度
* 使用说明: 适用于LwIP，low_level_input中调用，替代len = ;
*           获得数据包长度，接着分配pbuf
* 调用方法: len = enc28j60_packet_getlen();
********************************************************************************
*/

int enc28j60_packet_getlen(void)
{
	/* 数据包总长度 */
	int len = 0;
	int rxstat;

	/* 设置接收缓冲器读指针 */
	ENC28J60_Write(ERDPTL, (NextPacketPtr));
	ENC28J60_Write(ERDPTH, (NextPacketPtr) >> 8);

	/* 接收数据包结构示例 数据手册43页 */
	/* 读下一个包的指针 */
	NextPacketPtr = ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0) << 8;

	/* 读包的长度 */
	len = ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0);
	len |= ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0) << 8;

	/* 删除CRC计数 */
	len -= 4;

	/* 读取接收状态 */
	rxstat = ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0) << 8;

	/* 注意取消了CRC校验检查部分 */
	/* 返回接收数据包长度 */
	return len;
}

/*
********************************************************************************
* 函 数 名: enc28j60_finish_recevie
* 功能说明: 结束读取接收缓冲区过程，移动接收缓冲区指针
* 参    数: 无
* 返 回 值: 无
* 使用说明: 适用于LwIP，low_level_input中调用
*           acknowledge that packet has been read(); 或
*           drop packet();
* 调用方法: enc28j60_finish_receive()
********************************************************************************
*/
void enc28j60_finish_receive(void)
{
	/* 移动接收缓冲区 读指针*/
	ENC28J60_Write(ERXRDPTL, (NextPacketPtr));
	ENC28J60_Write(ERXRDPTH, (NextPacketPtr) >> 8);

	/* 数据包递减 */
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
}

//从网络获取一个数据包内容
//maxlen:数据包最大允许接收长度
//packet:数据包缓存区
//返回值:收到的数据包长度(字节)									  
u32 ENC28J60_Packet_Receive(u32 maxlen, u8* packet)
{
	u32 rxstat;
	u32 len;
	if (ENC28J60_Read(EPKTCNT) == 0)return 0;  //是否收到数据包?	   
											   //设置接收缓冲器读指针
	ENC28J60_Write(ERDPTL, (NextPacketPtr));
	ENC28J60_Write(ERDPTH, (NextPacketPtr) >> 8);
	// 读下一个包的指针
	NextPacketPtr = ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0) << 8;
	//读包的长度
	len = ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0);
	len |= ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0) << 8;
	len -= 4; //去掉CRC计数
			  //读取接收状态
	rxstat = ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM, 0) << 8;
	//限制接收长度	
	if (len>maxlen - 1)len = maxlen - 1;
	//检查CRC和符号错误
	// ERXFCON.CRCEN为默认设置,一般我们不需要检查.
	if ((rxstat & 0x80) == 0)len = 0;//无效
	else ENC28J60_Read_Buf(len, packet);//从接收缓冲器中复制数据包	    
										//RX读指针移动到下一个接收到的数据包的开始位置 
										//并释放我们刚才读出过的内存
	ENC28J60_Write(ERXRDPTL, (NextPacketPtr));
	ENC28J60_Write(ERXRDPTH, (NextPacketPtr) >> 8);
	//递减数据包计数器标志我们已经得到了这个包 
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return(len);
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
