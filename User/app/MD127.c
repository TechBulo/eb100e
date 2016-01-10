
#include "includes.h"

#include "motorconfig.h"

unsigned char test_127=0;  //相指针变量


static u8 iris_auto_table[4] = {0x05,0x06,0x0a,0x09};

void md127_step(u8 in1_fg,u8 in2_fg,u8 in3_fg,u8 in4_fg)//相转换函数
{

    if(in1_fg)
    	GPIO_SetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_A1);	
    else
    	GPIO_ResetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_A1);	
    
    if(in2_fg)
    	GPIO_SetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_A2);	
    else
    	GPIO_ResetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_A2);	
    
    if(in3_fg)
    	GPIO_SetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_B1);	
    else
    	GPIO_ResetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_B1);	
    
    if(in4_fg)
    	GPIO_SetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_B2);	
    else
    	GPIO_ResetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_B2);	
    
}

void md127_standby(void) //钳住相函数
{
	GPIO_ResetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_A1);	
	GPIO_ResetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_A2);	
	GPIO_ResetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_B1);	
	GPIO_ResetBits(IRIS_AUTO_PORT,IRIS_AUTO_PIN_B2);	
}

void md127_keepmode(void)//保持上一输入状态函数
{
    u16 temp;

    temp = GPIO_ReadOutputData(IRIS_AUTO_PORT);
    temp &= 0xFFF0;
	temp |= ((u16)0X0F);

	GPIO_Write(IRIS_AUTO_PORT, temp);

}


#if 0
void md127_forward(void)//正向前进函数
{
    switch(test_127)
    {
     case 0:
     md127_step(1,0,1,0); //a
     md127_keepmode();
     break;
     case 1:
     md127_step(0,1,1,0);//6
     md127_keepmode();
     break;
     case 2:
     md127_step(0,1,0,1);//5
     md127_keepmode();
     break;

     case 3:
     md127_step(1,0,0,1);//9
     md127_keepmode();
     break;
/*   
     case 3:
     md127_step(0,1,1,0);
     md127_keepmode();
     break;
     case 4:
     md127_step(0,1,0,0);
     md127_keepmode();
     break;
     case 5:
     md127_step(0,1,0,1);
     md127_keepmode();
     break;
     case 6:
     md127_step(0,0,0,1);
     md127_keepmode();
     break;
     case 7:
     md127_step(1,0,0,1);
     md127_keepmode();
     break;
     */
     default:break;
     }
 test_127++;
 if(test_127>3)
 test_127=0;
 }
#else
void md127_forward(void)//正向前进函数
{
    u16 temp;
    temp = GPIO_ReadOutputData(IRIS_AUTO_PORT);
    temp &= 0xFFF0;
	temp |= ((u16)iris_auto_table[test_127]);

	GPIO_Write(IRIS_AUTO_PORT, temp);

    test_127++;
    if(test_127>3)
    test_127=0;
 
}
#endif


#if 0
void md127_reverse(void)//反向函数
{
    switch(test_127)
    {
    case 3:
     md127_step(1,0,1,0);
     md127_keepmode();
     break;
     case 2:
     md127_step(0,1,1,0);
     md127_keepmode();
     break;
     case 1:
     md127_step(0,1,0,1);
     md127_keepmode();
     break;

     case 0:
     md127_step(1,0,0,1);
     md127_keepmode();
     break;
    default:break;
    }
 test_127++;
 if(test_127>7)
 test_127=0;
 }

#else
void md127_reverse(void)//反向函数
{
u16 temp;

    temp = GPIO_ReadOutputData(IRIS_AUTO_PORT);
    temp &= 0xFFF0;
	temp |= ((u16)iris_auto_table[test_127]);

	GPIO_Write(IRIS_AUTO_PORT, temp);

 test_127++;
 if(test_127>7)
 test_127=0;
 }

#endif

void iris_auto_motor_run(u8 mode)
{
    static u8 i=0;

	u16 temp = 0;

	
	switch(mode)
	{
	case 1:
    	temp = GPIO_ReadOutputData(IRIS_AUTO_PORT);
        temp &= 0xFFF0;
    	temp |= ((u16)iris_auto_table[i]);

		GPIO_Write(IRIS_AUTO_PORT, temp);

		i++;
		if(i>=4)
			i = 0;

        zoom_run_state_pre = zoom_run_state;
        zoom_run_state = 1;
		break;
	case 2:

    	temp = GPIO_ReadOutputData(IRIS_AUTO_PORT);
        temp &= 0xFFF0;
    	temp |= ((u16)iris_auto_table[i]);

		GPIO_Write(IRIS_AUTO_PORT, temp);
		//i--;
		 if(i == 0)
			i = 4-1;
		else
			i--;

        zoom_run_state_pre = zoom_run_state;
        zoom_run_state = 1;
		
		break;
	case 0:
	
        md127_keepmode();
        zoom_run_state_pre = zoom_run_state;
        zoom_run_state = 0;
      
		break;
	default:
		return;
	}

    md127_keepmode();
}

void iris_auto_forward(void)
{

}

void iris_auto_handle(void)
{
    

}

