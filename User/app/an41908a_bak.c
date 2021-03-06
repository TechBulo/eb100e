/****************************************************************************************************************
File :	Lensdrv.c
Function	 :	Lens Driver Ctrl For AN41908A
Made by	 :	2011/10/12	PSISZ DC
Description	 :
Env :	Keil51 C Compiler
Copyright:	MATSUSHITA ELECTRIC INDUSTRIAL CO.,LTD.
****************************************************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

#include "includes.h"
#include "AN41908A.h"

#define LENS_SUNNY 0



#define SPI_CS_AN41908A_PIN        GPIO_Pin_4//PA4
#define SPI_CS_AN41908A_HIGH        GPIO_WriteBit(GPIOA, SPI_CS_AN41908A_PIN, Bit_SET)
#define SPI_CS_AN41908A_LOW        GPIO_WriteBit(GPIOA, SPI_CS_AN41908A_PIN, Bit_RESET)

#define SPI_SCK_AN41908A_PIN        GPIO_Pin_5//PA5
#define SPI_SCK_AN41908A_HIGH        GPIO_WriteBit(GPIOA, SPI_SCK_AN41908A_PIN, Bit_SET)
#define SPI_SCK_AN41908A_LOW        GPIO_WriteBit(GPIOA, SPI_SCK_AN41908A_PIN, Bit_RESET)


#define SPI_SOUT_AN41908A_PIN        GPIO_Pin_6//PA6
#define SPI_SOUT_AN41908_PIN_GET    GPIO_ReadOutputDataBit(GPIOA, SPI_SOUT_AN41908A_PIN)

#define SPI_SIN_AN41908A_PIN        GPIO_Pin_7
#define SPI_SIN_AN41908A_MODE(VAL)  GPIO_WriteBit(GPIOA, SPI_SIN_AN41908A_PIN,VAL)

#define SPI_VD_IS_AN41908A_PIN      GPIO_Pin_4
#define SPI_VD_IS_AN41908A_PIN_MODE(VAL)  GPIO_WriteBit(GPIOB, SPI_VD_IS_AN41908A_PIN,VAL)


#define SPI_VD_FZ_AN41908A_PIN      GPIO_Pin_3
#define SPI_VD_FZ_AN41908A_PIN_MODE(VAL)  //GPIO_WriteBit(GPIOB, SPI_VD_FZ_AN41908A_PIN,VAL)

void delay_10ns(void)
{
    __nop();

}

void delay_100ns(void)
{
    __nop();
    __nop();

    __nop();
    __nop();

    __nop();
    __nop();
    __nop();
    __nop();

}


void an41908a_spi_pin_init(void)
{
	GPIO_InitTypeDef GPIOD_InitStructure;

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIOD_InitStructure.GPIO_Pin = SPI_CS_AN41908A_PIN|SPI_SCK_AN41908A_PIN|SPI_SIN_AN41908A_PIN;
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOD_InitStructure);

	GPIOD_InitStructure.GPIO_Pin = SPI_SOUT_AN41908A_PIN;
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOD_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	GPIOD_InitStructure.GPIO_Pin = SPI_VD_FZ_AN41908A_PIN;
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIOD_InitStructure);

}

#if 1
#define LENSDRV_FOCUS_STEPS_REG  0x24 //define AB motor to Focus
#define LENSDRV_ZOOM_STEPS_REG 	0x29   //define CD motor to Zoom

#else
#define LENSDRV_FOCUS_STEPS_REG   0x29 //define AB motor to Focus
#define LENSDRV_ZOOM_STEPS_REG 	0x24   //define CD motor to Zoom
#endif

/***********************************************************************
#ifdef	DOC
FUNCTION	: LensDrvInit
DESCRIPTION	: Initial Lens Driver's registor
INPUT		: none
OUTPUT		: none
UPDATE		:
DATE		: 2011/02/21
#endif
***********************************************************************/
void LensDrvInit( void )
{
	LensDrvFocusZoomInit();
	//LensDrvIrisInit();
}


/***********************************************************************
#ifdef	DOC
FUNCTION	: LensDrvSPIWriteUint
DESCRIPTION	: write Lens driver's registor
INPUT		: addr,wt_data
OUTPUT		: none
UPDATE		:
DATE		: 2011/02/21
#endif
***********************************************************************/
void LensDrvSPIWriteUint( uchar addr, uint wt_data )
{
	uchar i,j=0;

	SPI_CS_AN41908A_LOW;
    
	delay_100ns();
	SPI_SCK_AN41908A_HIGH;
	SPI_CS_AN41908A_HIGH;
  
    for ( i = 0; i <8; i++)
	{
        delay_100ns();
        SPI_SCK_AN41908A_LOW;
        delay_10ns();
		if( i<= 5)
		{
		  		SPI_SIN_AN41908A_MODE((addr&(1<<i)) >> i );
		} 
		else
	 	{
			SPI_SIN_AN41908A_MODE(0);  //set write bit
		}
		 delay_100ns();
		SPI_SCK_AN41908A_HIGH;
		 delay_100ns();
	}
   
	/*write input data*/
	for ( i = 0; i <16; i++)
	{
		 delay_10ns();
		SPI_SCK_AN41908A_LOW;
		delay_10ns();
		SPI_SIN_AN41908A_MODE( ( wt_data&(1<<i) )>>i);
		delay_10ns();
		SPI_SCK_AN41908A_HIGH;;
		delay_100ns();
	}


	delay_100ns();
	SPI_CS_AN41908A_LOW; //set stop bit
	delay_100ns();
}

/***********************************************************************
#ifdef	DOC
FUNCTION	: LensDrvSPIReadUint
DESCRIPTION	: Read the value of Lens driver's registor 
INPUT		: addr
OUTPUT		: value of Lens driver's registor(addr)
UPDATE		:
DATE		: 2011/02/21
#endif
***********************************************************************/
uint LensDrvSPIReadUint( uchar addr )
{
    uint tmp_rddata1= 0,tmp_rddata2 = 0;
	uchar i,j=0;
	SPI_CS_AN41908A_LOW;
	delay_100ns();
	SPI_SCK_AN41908A_HIGH;
	SPI_CS_AN41908A_HIGH;

	for ( i = 0; i <8; i++)
	{
		delay_10ns();
		SPI_SCK_AN41908A_LOW;
		delay_10ns();
		if( i<= 5)
		{
		  		SPI_SIN_AN41908A_MODE( (addr & (1<<i)) >> i );
		} 
		else
	 	{
			SPI_SIN_AN41908A_MODE(1);  //set read bit
		}
		delay_10ns();
		SPI_SCK_AN41908A_HIGH;
		delay_100ns();
		//delay_100ns();
	}

	delay_10ns();
	/*read data*/
	for ( i = 0; i <16; i++)
	{
		delay_10ns();
		SPI_SCK_AN41908A_LOW;
		delay_100ns();
		SPI_SCK_AN41908A_HIGH;
		delay_10ns();
		tmp_rddata1 = SPI_SOUT_AN41908_PIN_GET;
		tmp_rddata1 = tmp_rddata1 << i;
		tmp_rddata2 |= tmp_rddata1;
		delay_10ns();
	}
	delay_10ns();
	SPI_CS_AN41908A_LOW;	//set stop bit
	delay_10ns();
	return tmp_rddata2;

}

/***********************************************************************
#ifdef	DOC
FUNCTION	: LensDrvFocusZoomInit
DESCRIPTION	: Initial Focus&Zoom stepping motor potion
INPUT		: none
OUTPUT		: none
UPDATE		:
DATE		: 2011/02/21
#endif
***********************************************************************/
void LensDrvFocusZoomInit( void )
{

#if LENS_SUNNY //Zoom use AB chanel
	LensDrvSPIWriteUint(0x20,0x5c0a);
	LensDrvSPIWriteUint(0x21,0x0000);
	LensDrvSPIWriteUint(0x22,0x0003);
	LensDrvSPIWriteUint(0x23,0xC8C8);
	LensDrvSPIWriteUint(0x24,0x0400);
	LensDrvSPIWriteUint(0x25,0x0502);
#else
	LensDrvSPIWriteUint(0x20,0x5c0a);
	LensDrvSPIWriteUint(0x21,0x0000);
	LensDrvSPIWriteUint(0x22,0x1603);
	LensDrvSPIWriteUint(0x23,0xC8C8);
	LensDrvSPIWriteUint(0x24,0x0400);
	LensDrvSPIWriteUint(0x25,0x0160);
 #endif

	LensDrvSPIWriteUint(0x27,0x1603);
	LensDrvSPIWriteUint(0x28,0xC8C8);
	LensDrvSPIWriteUint(0x29,0x0400);
	LensDrvSPIWriteUint(0x2A,0x0400);
}

/***********************************************************************
#ifdef	DOC
FUNCTION	: LensDrvIrisInit
DESCRIPTION	: Initial Iris potion
INPUT		: none
OUTPUT		: none
UPDATE		:
DATE		: 2011/02/21
#endif
***********************************************************************/
void LensDrvIrisInit( void )
{
#if LENS_SUNNY
	LensDrvSPIWriteUint(0x00,0x0000);  //Set Iris Target
	LensDrvSPIWriteUint(0x01,0x808A);  
	LensDrvSPIWriteUint(0x02,0x66F0);
	LensDrvSPIWriteUint(0x03,0x0E10);
	LensDrvSPIWriteUint(0x04,0x7E20);
	LensDrvSPIWriteUint(0x05,0x0A04);
	LensDrvSPIWriteUint(0x0A,0x0000);
	LensDrvSPIWriteUint(0x0B,0x0400);
	LensDrvSPIWriteUint(0x0E,0x0C00);

#else
	LensDrvSPIWriteUint(0x00,0x0000);  //Set Iris Target
	LensDrvSPIWriteUint(0x01,0x6600);  
	LensDrvSPIWriteUint(0x02,0x5400);
	LensDrvSPIWriteUint(0x03,0x0E10);
	LensDrvSPIWriteUint(0x04,0x8437);
	LensDrvSPIWriteUint(0x05,0x0104);
	LensDrvSPIWriteUint(0x0A,0x0042);
	LensDrvSPIWriteUint(0x0B,0x0400);
	LensDrvSPIWriteUint(0x0E,0x0D00);
#endif
}

/***********************************************************************
#ifdef	DOC
FUNCTION	: LensDrvIrisMove
DESCRIPTION	: Iris move control
INPUT		: IrsTGT
OUTPUT		: none
UPDATE		:
DATE		: 2011/02/21
#endif
***********************************************************************/
//after implement this function,there should be VD_IS pluse. 
void LensDrvIrisMove(uint IrsTGT)
{
	//IrsTGT range:0x0000~0x03FF
	if ( IrsTGT> 0x03ff )
	{
		return ;
	}
	
	LensDrvSPIWriteUint(0x00,IrsTGT);  //Set Iris Target
#if 1	//output a VD_IS pluse
	SPI_VD_IS_AN41908A_PIN_MODE(0);
	LensdrvDelay_us(1);
	SPI_VD_IS_AN41908A_PIN_MODE(1);
#endif 
}

/***********************************************************************
#ifdef	DOC
FUNCTION	: LensDrvFocusMove
DESCRIPTION	: Focus motor move control
INPUT		: dir,FcStep
OUTPUT		: none
UPDATE		:
DATE		: 2011/02/21
#endif
***********************************************************************/
//after implement this function,there should be VD_FZ pluse. 
void LensDrvFocusMove(u8 dir, uint FcStep)
{
	uint backup_data = 0;
	if ( FcStep > 0x00FF) // more than 255step
	{	
		return;
	}
	backup_data = LensDrvSPIReadUint( LENSDRV_FOCUS_STEPS_REG );
	backup_data &=0xFE00;
	backup_data |=FcStep;
	if ( dir == 1)
	{  
	    //Add directiion bit (CCWCWAB) to registor:0x24
		backup_data |=0x0100;
		LensDrvSPIWriteUint( LENSDRV_FOCUS_STEPS_REG,backup_data);
	} 
	else
	{
		LensDrvSPIWriteUint( LENSDRV_FOCUS_STEPS_REG,backup_data);
	}
#if 1	//output a VD_FZ pluse
	SPI_VD_FZ_AN41908A_PIN_MODE(1);
	LensdrvDelay_us(1);
	SPI_VD_FZ_AN41908A_PIN_MODE(0);
#endif 
}

/***********************************************************************
#ifdef	DOC
FUNCTION	: LenDrvZoomMove
DESCRIPTION	: Zoom motor move control
INPUT		: dir,ZoomStep
OUTPUT		: none
UPDATE		:
DATE		: 2011/02/21
#endif
***********************************************************************/
//after implement this function,there should be VD_FZ pluse. 
void LenDrvZoomMove( u8 dir, uint ZoomStep)   
{
	uint backup_data = 0;
	if ( ZoomStep > 0x00FF) // more than 255step
	{	
		return;
	}
	backup_data = LensDrvSPIReadUint( LENSDRV_ZOOM_STEPS_REG );
	backup_data &=0xFE00;
	backup_data |=ZoomStep;
	if ( dir == 1)
	{  
	    //Add directiion bit (CCWCWAB) to registor:0x29
		backup_data |=0x0100;
		LensDrvSPIWriteUint( LENSDRV_ZOOM_STEPS_REG,backup_data);
	} 
	else
	{
		LensDrvSPIWriteUint( LENSDRV_ZOOM_STEPS_REG,backup_data);
	}
#if 1	//output a VD_FZ pluse
	SPI_VD_FZ_AN41908A_PIN_MODE(1);
	LensdrvDelay_us(1);
	SPI_VD_FZ_AN41908A_PIN_MODE(0);
#endif 
}



void LensdrvDelay_us(unsigned int x)
{
	unsigned int i,j;
	for (i=0;i<x;i++)
	   for (j=0;j<72;j++)
		   ;
}

#define AN41908_LED_A_REG       0X29
#define AN41908_LED_B_REG       0X24


void an41908_led_mode_set(u8 mode)
{
	uint backup_data = 0;

	backup_data = LensDrvSPIReadUint( LENSDRV_ZOOM_STEPS_REG );
	if ( mode)
	{  
	    //Add directiion bit (CCWCWAB) to registor:0x29
		backup_data |=0x0080;
		LensDrvSPIWriteUint( AN41908_LED_A_REG,backup_data);
	} 
	else
	{
	    backup_data &=~0x0080;
		LensDrvSPIWriteUint( AN41908_LED_A_REG,backup_data);
	}
#if 1	//output a VD_FZ pluse
	SPI_VD_FZ_AN41908A_PIN_MODE(1);
	LensdrvDelay_us(1);
	SPI_VD_FZ_AN41908A_PIN_MODE(0);
#endif 
}


