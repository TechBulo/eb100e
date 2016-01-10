/*
 * FileName:       
 * Author:     909220579qq.com      
 * Date: 2013-11-10
 * Description:  M25P80 驱动程序
 * Version:        
 * Function List:使用stm32上的spi与flash m25p80的数据读写，具体情况更改硬件连接即可  
 *      
 * History:            
 */
#include "includes.h"
#include "Spi.h"       

#include "SpiFF.h" 


#define SPI_Flash_Read() SPI_ReadByte(SPI1)
#define	SPI_Flash_Write(x) SPI_WriteByte(SPI1,x)

u8 SPI_FLASH_SendByte(u8 byte)
{
  /* Loop while DR register in not emplty */
  //while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
   while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
 // /* Send byte through the SPI1 peripheral */
 // SPI_SendData(SPI1, byte);
   /* Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, byte);
  /* Wait to receive a byte */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}
void M25P80_Write_Enable(void)
{
	Select_Flash();	
	SPI_Flash_Write(WRITE_ENABLE);	
	NotSelect_Flash();
}

void M25P80_Write_Disable(void)
{
	Select_Flash();	
	SPI_Flash_Write(WRITE_DISABLE);	
	NotSelect_Flash();
}

void M25P80_Read_Id(u8 * id)
{
	u8 i;	
	Select_Flash();	
	SPI_Flash_Write(READ_ID);	
	for(i=0;i<3;i++)
	{
		id[i] = SPI_Flash_Read();	
	}	
	NotSelect_Flash();
}

u8 M25P80_Read_Status_Reg(void)
{
	u8 sta;
	Select_Flash();	
	SPI_Flash_Write(READ_STAUS_REG);	
	sta= SPI_Flash_Read();	
	NotSelect_Flash();	
	return sta;
}

void M25P80_Write_Status_Reg(u8 reg)
{
	Select_Flash();	
	SPI_Flash_Write(WRITE_STAUS_REG);	
	SPI_Flash_Write(reg);
	NotSelect_Flash();
}

//读数据，自动翻页,addr为字节地址
void M25P80_Read_Data(u32 addr,u32 len,u8 *buf)
{
	u32 i;	
	Select_Flash();	
	SPI_Flash_Write(READ_DATA);	
	SPI_Flash_Write((addr>>16) & 0xff);//扇区：16个
	SPI_Flash_Write((addr>>8) & 0xff); //页：256页
	SPI_Flash_Write(addr & 0xff);//页内地址：256字节
	
	for(i=0;i<len;i++)
	{
	  buf[i]=SPI_Flash_Read();
	}
	NotSelect_Flash();
}

//快速读数据
void M25P80_Fast_Read_Data(u32 addr,u32 len,u8 *buf)
{
	u32 i;

	Select_Flash();	
	SPI_Flash_Write(FAST_READ_DATA);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);
	SPI_Flash_Write(0);
	
	for(i=0;i<len;i++)
	{
		buf[i]=SPI_Flash_Read();
	}
	NotSelect_Flash();
}

//页编程函数，页编程前一定要进行区擦除!
void M25P80_Page_Program(u32 addr,u16 len,u8 *buf)
{
	u32 i;	
	M25P80_Write_Enable();
	Select_Flash();	
	SPI_Flash_Write(PAGE_PROGRAM);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);

	for(i=0;i<len;i++)
		SPI_Flash_Write(buf[i]);
	NotSelect_Flash();
	while(M25P80_Read_Status_Reg()&0x01);
    //NotSelect_Flash();	
}

void M25P80_write_byte(u32 addr,u8 data)
{

	M25P80_Page_Program(addr,1,&data);

}

void M25P80_write_buff(u32 addr,u8 *data,u32 len)
{

	M25P80_Page_Program(addr,len,data);

}



void M25P80_Sector_Erase(u32 addr)
{
	M25P80_Write_Enable();
	Select_Flash();	
	SPI_Flash_Write(SECTOR_ERASE);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);
	NotSelect_Flash();
	while(M25P80_Read_Status_Reg()&0x01);
	//NotSelect_Flash();
}

void M25P80_Bulk_Erase(void)
{
	M25P80_Write_Enable();
	Select_Flash();	
	SPI_Flash_Write(BULK_ERASE);	
	NotSelect_Flash();
	while(M25P80_Read_Status_Reg()&0x01);
	//NotSelect_Flash();
}

void M25P80_Deep_Power_Down(void)
{
	Select_Flash();	
	SPI_Flash_Write(DEEP_POWER_DOWN);	
	NotSelect_Flash();
}

u8 M25P80_Wake_Up(void)
{
	u8 res;
	Select_Flash();	
	SPI_Flash_Write(WAKE_UP);	
	SPI_Flash_Write(0);	
	SPI_Flash_Write(0);	
	SPI_Flash_Write(0);	
	res=SPI_Flash_Read();	
	NotSelect_Flash();
	return res;
}


u8 eeprom_byte_read(u32 address)
{
	u8 data;
	M25P80_Read_Data(address,1,&data);

	return data;
}

void eeprom_byte_write(u32 address, u8 data)
{
	M25P80_write_buff(address,&data,1);
}


u8 user_buff[50];
void M25P80_Init(void)
{
	SPI1_Init();


	GPIO_InitTypeDef GPIOD_InitStructure;

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOD_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET); //将AN41908的CS拉高，关闭

		GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_RESET); //将AN41908的CS拉高，关闭

	NotSelect_Flash();
	Select_Flash();	
	delay_X1ms(200);
	
	while(1)
	{
		M25P80_Read_Id(user_buff);
		if(user_buff[0]!= 0)
			break;

		delay_X1ms(200);
	}
}
