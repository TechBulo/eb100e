#include "includes.h"
#include "Protocol.h"
#include "Queue.h"
#include "malloc.h"
#include <stddef.h>

//#include "eeprom.h"




uchar Isr_i=0;
uchar Isr_com1=0;
uchar Isr_j=0;
static uchar Isr_com;



uchar *keyboard_data_buffer_point;
uchar keyboard_data_buffer[20];
uchar Pelco_fir_data;                       //当前协议的命令串的第一个字节
uchar Rec_byte_count;
uchar cmd_type;
uchar cmd_flag;   
uchar keyboard_data_history[2];
uchar x_his_speed;
uchar y_his_speed;
uchar Rec_byte_count_buff;
uchar Rec_spe_byte_count_buff;
uchar ultrak_erna_rec_tour_data_f;
uchar pan_direction;
uchar tilt_direction;
uchar pan_tilt_flag;

void return_cmd_to_camlink(u8 cmd,u8 para);

void uart0_rx_isr(uchar udr0)
{
    uchar i;
    static uchar keyboard_data_buffer1[20];  

	ProUart0Rec = 0;
	{ 
		Isr_i = udr0;

	  	KBDRecWrongFlag = 0;
	  	if (0x00 == Isr_j) 
		{
			if (0x01 == Current_Protocol) 
			{
				switch (Isr_i) 
				{
				 case 0xff: Rec_byte_count = 0x07;
				            Rec_byte_count_buff = 0x07;
				            Rec_spe_byte_count_buff = 0x09;
							Protocol_No = PELCO_D;
				            break;
				 case 0xa0: Rec_byte_count = 0x08;
				            Rec_spe_byte_count_buff = 0x0a;
				            Rec_byte_count_buff = 0x08;
							Protocol_No = PELCO_P;
							break;	
                case CAMERA_LINK_CMD_HEAD:
                    Rec_byte_count = 8;
                    Rec_spe_byte_count_buff = 0x0a;
                    Rec_byte_count_buff = 0x08;
                    Protocol_No = PROTOCOL_CAM_LINK;
                    break;	            
				 default:   
					Isr_com = 0; 
						Isr_j = 0;
					return;
				}
			}
			else
			{
				switch (Protocol_No)
				{
				case PELCO_D: 
					if(PELCO_D_FRAME_HEADE !=Isr_i)
					{
						Isr_com = 0; 
						Isr_j = 0;
						return;
					}
					break;
				case PELCO_P: 
					if(PELCO_P_FRAME_HEADE !=Isr_i)
					{
						Isr_com = 0; 
						Isr_j = 0;
						return;
					}
					break;
				default:

					break;
				}
			}
			keyboard_data_buffer1[0] = Isr_i;
			Isr_com = 1; 
			Isr_j = 0x01;
		}
		else 
		{                   //命令的第二个字节
			keyboard_data_buffer1[Isr_com] = Isr_i;
			Isr_com++;

			switch (Protocol_No)
			{
			case PELCO_P: 
			case PELCO_D: //if (0x07 == Rec_byte_count)
				Rec_byte_count = Rec_byte_count_buff;  //    
				break;
 			case PROTOCOL_CAM_LINK: 
				Rec_byte_count = Rec_byte_count_buff;  //    
				break;
               
			default:
                
				break;
			}
	
			if (Isr_com >= Rec_byte_count)
			{
				for (i=0x00; i<20; i++) 
					Rec_keyboard_data_buffer[i] = keyboard_data_buffer1[i];

				addqueue(20,keyboard_data_buffer1);

				for (i=0x00; i<20; i++) 
					keyboard_data_buffer1[i] = 0x00;
				if ((0xa0 == Rec_keyboard_data_buffer[0]) && (Protocol_No == PELCO_P))
				  Rec_keyboard_data_buffer[1]++;
				if (0x80 == (0xf0 & Rec_keyboard_data_buffer[0]))
				{
				i = Rec_keyboard_data_buffer[0] << 4;
				i |= Rec_keyboard_data_buffer[1] & 0x0f;
				}
				else i = Rec_keyboard_data_buffer[1];

				if (i == domeNo)
				{
					switch (Protocol_No)
					{
					case PELCO_P: 

						break;
					case PELCO_D: 
 
						break;
					default:
						break;
					}
			}
			Isr_com = 0x00; 
			Isr_j = 0x00; 
			Rec_byte_com = 0X01;	
						
			}	
		}
	}	
       
}

void System_data_init(void)
{
	my_mem_init();
  	Rec_byte_com=0x00;
  	command_byte=0x00;
  	PanTiltStopFlag = 0x00;
  	Rocket_fir_data=0x00;
  	Rocket_sec_data=0x00;
  	Rec_byte_count=0x00;
	Current_Protocol = 0x00; 
	if (0x00 == Protocol_No)
    Current_Protocol = 0x01;   //auto Identify



	switch (Protocol_No)
	   {
	    case PELCO_D:
		     Rec_byte_count = 0x07;
			 Rec_byte_count_buff = 0x07; 
			 Rec_spe_byte_count_buff = 0x09;
	         break;
	    case PELCO_P:
		     Rec_byte_count = 0x08; 
			 Rec_byte_count_buff = 0x08; 
			 Rec_spe_byte_count_buff = 0x0a;
	         break;
	    default:
			break; 	
	}
}


u8 cam_brightness_val = 2;
u8 cam_atw_level = 4;


u8 cam_percent_val_table[]={0,12,25,38,50,62,75,88,100};

u16 test1,test2;

static void camera_link_protocol_analysis(void)
{

    u8 cmd_tmp,para_tmp;
    u8 tmp;

    u16 tmp1,tmp2;
    
    command_byte=0xff;
    Rec_byte_com = 0x00;


	if (CAMERA_LINK_CMD_TAIL == keyboard_data_buffer[7])
    {
    	cmd_tmp = keyboard_data_buffer[2];
        para_tmp = keyboard_data_buffer[3];

        switch(cmd_tmp)
        {
        case 0xa1:
            return_cmd_to_camlink(0xa2,0x01);
            break;
        case 0xa8://透雾图像 控制相机切换为透雾滤片
            cam_filt_cutter_set(1);
            delay_X1ms(500);
            cam_color_mode_set(OFF_MODE);
            break;
        case 0xa5://彩色图像 控制相机切换为彩色滤片
            cam_filt_cutter_set(0);
            delay_X1ms(500);
            cam_color_mode_set(ON_MODE);
            break;
        case 0xad:// 增强模式，打开电子透雾功能 参数设置 1，2，3
            cam_frog_set(para_tmp);
            break;
        case 0xaa:// 电子透雾关闭
            cam_frog_set(0);
            break;
        case 0xb7://亮度增加
            tmp = cam_brightness_mode_get();

            //tmp = 2;
            if( cam_brightness_get(tmp,&cam_brightness_val))
            {
                if(cam_brightness_val<CAMERA_BRIGHTNESS_VAL_MAX)
                    cam_brightness_val++;
                cam_brightness_set(cam_brightness_val,tmp);
                return_cmd_to_camlink(0xb3,cam_percent_val_table[cam_brightness_val]);
            }
            break;
        case 0xb8://亮度减小
            tmp = cam_brightness_mode_get();
            //tmp = 2;
            if( cam_brightness_get(tmp,&cam_brightness_val))
            {
                if(cam_brightness_val > 0)
                    cam_brightness_val--;
                cam_brightness_set(cam_brightness_val,tmp);

                
                return_cmd_to_camlink(0xb3,cam_percent_val_table[cam_brightness_val]);
            }
            break;
        case 0xc2://彩色图像增益加
            cam_atw_level = cam_atw_level_get(0);

            if(cam_atw_level < 8)
                cam_atw_level++;
            cam_atw_level_set(cam_atw_level);
            return_cmd_to_camlink(0xb4,cam_percent_val_table[8-cam_atw_level]);
            break;
        case 0xc1://彩色图像增益减
            cam_atw_level = cam_atw_level_get(0);
            if(cam_atw_level > 0)
                cam_atw_level--;
            cam_atw_level_set(cam_atw_level);
            return_cmd_to_camlink(0xb4,cam_percent_val_table[8-cam_atw_level]);
            break;
        case 0xc3://数字变倍倍率  参数 0,close; 2,2x; 3,3x; 4,4x

            switch(para_tmp)
            {
            case 0:
                tmp = 100;
                break;
            case 2:
                tmp = 133;
                break;
            case 3:
                tmp = 167;
                break;
            case 4:
                tmp = 200;
                break;
            default:
                tmp = 100;
                break;
               
            }

            if(para_tmp)
            {
                cam_zoom_disp_set(ON_MODE);
                delay_X1ms(200);
                cam_zoom_mode_set(ON_MODE);
                delay_X1ms(200);
                cam_zoom_disp_set(OFF_MODE);
                
                
                
                #if 0
                delay_X1ms(200);
                cam_zoom_data_set(tmp);
                delay_X1ms(200);
                cam_zoom_data_set(tmp);
                
                cam_zoom_mode_set(OFF_MODE);
                //cam_zoom_mode_set(OFF_MODE);
                cam_zoom_data_set(tmp);
                cam_zoom_data_set(tmp);

                tmp2=20;
                while(tmp2--)
                {
                    cam_zoom_mode_set(ON_MODE);
                    delay_X1ms(20);
                    cam_zoom_data_set(tmp);
                    delay_X1ms(20);
                    if(cam_zoom_data_get(&tmp1))
                    {
                        if(tmp1 == tmp)
                        {
                            break;
                        }

                    }
                }
                #endif
                //cam_zoom_mode_set(ON_MODE);
            }
            
            if(!para_tmp)
            {
                cam_zoom_mode_set(OFF_MODE);
                delay_X1ms(50);
                cam_zoom_mode_set(OFF_MODE);
            }
            break;
        case 0xc4: //十字线 参数 0，关闭；1，打开
            cam_cross_line_mode_set(para_tmp?ON_MODE:OFF_MODE);
            break;
        case 0xc5://画面静止 参数 0，关闭；1，打开
            cam_freeze_set(para_tmp?ON_MODE:OFF_MODE);
            break;
        case 0xc7: //图像水平翻转开关0，关，1开
            switch(para_tmp)
            {
            case 0:
                cam_picture_reverse_set(PIC_NORMAL);
                break;
            case 1:
                cam_picture_reverse_set(PIC_MIRROR);
                break;
            case 2:
                cam_picture_reverse_set(PIC_VFLIP);
                break;
            case 3:
                cam_picture_reverse_set(PIC_ALL_FLIP);
                break;
            default:
                break;
            }
            break;
        case 0xc8: //图像垂直翻转开关0，关，1开

            break;
        case 0xcf://查询光圈值

            break;
        case 0xdf:// 关闭图像输出

            break;
        case 0xdc:// 打开图像输出

            break;
        case 0x01:// left
            Rocket_fir_data = keyboard_data_buffer[4];
	        command_byte = 0x06;         //pan left
            break;
        case 0x02://right
            command_byte = 0x07;         //pan right
	        Rocket_fir_data = keyboard_data_buffer[4];	
            break;
        case 0x03://up
            command_byte = 0x08;         // up
	        Rocket_sec_data = keyboard_data_buffer[5];
            break;
        case 0x04://down
            command_byte = 0x09;         // down
	        Rocket_sec_data = keyboard_data_buffer[5];
            break;
        case 0x05://enter
            command_byte=0x0e;          //OPEN
            break;
        case 0x09:// open osd menu
            command_byte=0x90;          //
            break;
        }
        
	}
}

u8 complex_cmd_flag=0;
u8 complex_cmd=0;







static u8 Rocket_fir_data_pre=0;

static void PELCO_D_P_protocol_analysis_2(void)
{
//	uchar KCPCom , k;
	uchar parity_byte;
	uchar Keyboard_data_com;
	uchar Pan_falg = 0;
	uchar Tilt_falg = 0;
//	static uchar Rec_tour_data_flag;          // 为1时开始接收TOUR数,为2时数据已接收完毕

	Keyboard_data_com = 0x00;
	parity_byte = 0xfe;                     //初始化parity_byte 与  Keyboard_data_com 不相等
	command_byte=0xff;
	Rec_byte_com = 0x00;

	if (keyboard_data_buffer[1] == domeNo || keyboard_data_buffer[1]==0xff) 
	{
		if ((keyboard_data_buffer[0] == 0xff) && (Protocol_No != PELCO_P))    
		{
			if (Rec_spe_byte_count_buff == Rec_byte_count)
			{ 
			   parity_byte = keyboard_data_buffer[1] + keyboard_data_buffer[2] + keyboard_data_buffer[3] + keyboard_data_buffer[4] +
			                 keyboard_data_buffer[5] + keyboard_data_buffer[6] + keyboard_data_buffer[7];
			   Keyboard_data_com = keyboard_data_buffer[8];
			}
			else 
			{
				parity_byte = keyboard_data_buffer[1] + keyboard_data_buffer[2] + keyboard_data_buffer[3] + keyboard_data_buffer[4] +
				           keyboard_data_buffer[5];
				Keyboard_data_com=keyboard_data_buffer[6];
		    } 
		}
		else if ((keyboard_data_buffer[0] == 0xa0) && (Protocol_No != PELCO_D)) 
		{
			if (Rec_spe_byte_count_buff == Rec_byte_count)
			{ 
				parity_byte = keyboard_data_buffer[0]^(keyboard_data_buffer[1]-1)^keyboard_data_buffer[2]^keyboard_data_buffer[3]^keyboard_data_buffer[4]^
				         keyboard_data_buffer[5]^keyboard_data_buffer[6]^keyboard_data_buffer[7]^keyboard_data_buffer[8];
				Keyboard_data_com = keyboard_data_buffer[9];
			}
			else 
			{
				parity_byte = keyboard_data_buffer[0]^(keyboard_data_buffer[1]-1)^keyboard_data_buffer[2]^keyboard_data_buffer[3]^
				                keyboard_data_buffer[4]^keyboard_data_buffer[5]^keyboard_data_buffer[6];
				Keyboard_data_com = keyboard_data_buffer[7];
			}
		}

		if (Keyboard_data_com == parity_byte)
		{
			
			if (0x75 == keyboard_data_buffer[2])//PAN相对位置
			{
				command_byte = 0x90;
				pan_tilt_flag = 1;
				Rocket_fir_data = keyboard_data_buffer[4];
				Rocket_sec_data = keyboard_data_buffer[5];
				Rocket_fiv_data = keyboard_data_buffer[3];//speed 
				pan_direction = Rocket_fiv_data & 0x80;
			
			}
			else if (0x76 == keyboard_data_buffer[2])//TITL相对位置
			{
				command_byte = 0x90;
				pan_tilt_flag = 2;
				Rocket_thr_data = keyboard_data_buffer[4];
				Rocket_fou_data = keyboard_data_buffer[5];
				Rocket_fiv_data = keyboard_data_buffer[3];//speed
				tilt_direction = Rocket_fiv_data & 0x80;
					
			}
			else if (0x77 == keyboard_data_buffer[2])//ZOOM相对倍率(1-100)
			{
				command_byte = 0x91;
				Rocket_fir_data = keyboard_data_buffer[4];//方向
				Rocket_sec_data = keyboard_data_buffer[5];//倍值
				
			}
			else if (0x20 == keyboard_data_buffer[2])//PAN and TITL相对位置
			{
				command_byte = 0x90;
				Rocket_fiv_data = keyboard_data_buffer[7];//speed and direction
				
				if((!keyboard_data_buffer[3])&&(!keyboard_data_buffer[4]))
				{
					Pan_falg = 0;
				}
				else  //PAN 有动作
				{
					Pan_falg = 1;
					Rocket_fir_data = keyboard_data_buffer[3];
					Rocket_sec_data = keyboard_data_buffer[4];
					pan_direction = Rocket_fiv_data & 0x80;
				}
				
				if((!keyboard_data_buffer[5])&&(!keyboard_data_buffer[6]))
				{
					Tilt_falg = 0;
				}
				else  //TITL 有动作
				{
					Tilt_falg = 2;
					Rocket_thr_data = keyboard_data_buffer[5];
					Rocket_fou_data = keyboard_data_buffer[6];
					tilt_direction = (Rocket_fiv_data & 0x40)<<1;					
				}
				pan_tilt_flag = Pan_falg | Tilt_falg;						
			}

			else
				if(!keyboard_data_buffer[3])
				{
					switch(keyboard_data_buffer[2])
				   	{
					case 0x01: command_byte=0x04;        //NEAR
						break;			
					case 0x02: command_byte=0x0e;          //OPEN
						break;
					case 0x04: command_byte=0x0f;         //CLOSE
						break;
					case 0x00:
						command_byte=0x05;         //stop
						jiguang_zoom_state = 1;
						break;	

					case 0x03:

						break;

					case 0x05:

						break;

					case 0x06:

						break;

					default:
						break;

					}
				}
				else
				{
				   	switch(keyboard_data_buffer[2])
				   	{

					case 0x1d:
					//ReturnVersion(0);
						command_byte = 0xff;
						break;

					case 0x40://360度自动扫描  特殊功能编号command_byte:0X80-0XA0
						Rocket_sec_data = keyboard_data_buffer[5];
						command_byte = 0x80;
						break;
					case 0x45:
						
						break;
					case 0x50://run/stop tour
						if(0x01 == keyboard_data_buffer[3])
						{
							Rocket_sec_data = keyboard_data_buffer[5];
							command_byte = 0x82;
						}
						else if(0x02 == keyboard_data_buffer[3])
						{
							command_byte = 0x83;//stop all tour
						}
						break;
					case 0x51://record tour
						Rocket_sec_data = keyboard_data_buffer[5];
						command_byte = 0x84;

						break;
					case 0x52://preset pic still
						break;

					case 0x53:

						break;
					case 0x61:
						break;
					case 0x62:
						break;

					default:
                        if(keyboard_data_buffer[3] != 0x03)
                        {
                            complex_cmd_flag = 0;
                        }
                        
						switch (keyboard_data_buffer[3])
						{
						case 0x20: command_byte = 0x01;     //放大
						        break;
						case 0x40: command_byte = 0x02;     //缩小
						        break;
						case 0x80: command_byte = 0x03;        //FAR    for pec_D
						        break;
						case 0x04: Rocket_fir_data = keyboard_data_buffer[4];
						        command_byte = 0x06;         //pan left
						        break;
						case 0x02:	command_byte = 0x07;         //pan right
						        Rocket_fir_data = keyboard_data_buffer[4];	
								break;
						case 0x08:	command_byte = 0x08;         // up
						        Rocket_sec_data = keyboard_data_buffer[5];
						        break;
						case 0x10:	command_byte = 0x09;         // down
						        Rocket_sec_data = keyboard_data_buffer[5];
						        break;	
						case 0x0c:	command_byte = 0x0a;         // left up
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
						        break;
						case 0x14: command_byte = 0x0b;          //left down
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];		
								break;
						case 0x0a: command_byte = 0x0c;          //right up
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;
						case 0x12: command_byte = 0x0d;          //right down
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;	
                        case 0x07: command_byte = 0x11;                           //呼叫预置点        
						
						        Rocket_fir_data = keyboard_data_buffer[5]; 	
								break;
						case 0x03: command_byte = 0x10;                            //设置预置点
						        Rocket_fir_data = keyboard_data_buffer[5]; 

                                
                                    switch(complex_cmd_flag)
                                    {
                                    case 0:
                                        if(Rocket_fir_data == 255)
                                        {
                                            complex_cmd_flag = 1;
                                            
                                        }
                                        else
                                        {
                                            complex_cmd = 0; //set default system parameter
                                            complex_cmd_flag = 0;
                                        }
                                        break;
                                    case 1:
                                        if((Rocket_fir_data == 254 ))
                                        {
                                            complex_cmd_flag = 2; //
                                            
                                        }
                                        else if(Rocket_fir_data == 253)
                                        {
                                            complex_cmd = 1; //set default system parameter
                                            complex_cmd_flag = 2;
                                        }
									    else if(Rocket_fir_data == 255)
                                        {//设置ID 地址
                                            complex_cmd = 2; //set domeid
                                            complex_cmd_flag = 2;
                                        }
                                        else if(Rocket_fir_data >= 205 && Rocket_fir_data<=208)
                                        {
                                            save_system_para(Rocket_fir_data-200);
                                            
                                            complex_cmd = 0; //set default system parameter
                                            complex_cmd_flag = 0;
                                        }
                                         else
                                        {
                                            complex_cmd = 0; //set default system parameter
                                            complex_cmd_flag = 0;
                                        }
                                            
                                        break;
                                    case 2:
                                        
                                        Rocket_fir_data_pre = Rocket_fir_data;
                                        complex_cmd_flag = 3;
                                        
                                        break;
                                    case 3:

                                        if(complex_cmd == 1)
                                        {
                                            if((Rocket_fir_data == 253 ))
                                            {
                                                if(Rocket_fir_data_pre>=201 && Rocket_fir_data_pre<=204)
                                                save_system_para(Rocket_fir_data_pre-200);
                                            }
                                            complex_cmd = 0;
                                        }
                                        else
                                        {
                                            if((complex_cmd == 2 ))
                                            {
                                                domeNo = Rocket_fir_data_pre;
                                                save_domeID(domeNo);
                                            }
                                            else if((Rocket_fir_data == 254 ))
                                            {
                                                Baud_rate = Rocket_fir_data;
                                                if(Baud_rate>6)
                                                    Baud_rate = 2;
                                                save_baudrate(Baud_rate);
                                                
                                            }

                                            complex_cmd_flag = 0;
                                        complex_cmd = 0;
                                        }
                                         
                                        break;
                                    default:
                                        complex_cmd_flag = 0;
                                        complex_cmd = 0;
                                        break;
                                    }

                                
								break;
						case 0x05: command_byte = 0x12;                          //删除预置点
						        Rocket_fir_data = keyboard_data_buffer[5];  	
								break;
						case 0x4d: command_byte = 0x13;                          //清除轮巡轨迹
						        Rocket_fir_data = keyboard_data_buffer[5];
								break;
						case 0x51: command_byte = 0x14;                          //停止轮巡轨迹
						        Rocket_fir_data = keyboard_data_buffer[5];
								break;
						case 0x4f: command_byte = 0x15;                          //呼叫轮巡轨迹
						        Rocket_fir_data = keyboard_data_buffer[5];
								break;
						case 0x24: command_byte = 0x17;          //left zoom in
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;	
						case 0x22: command_byte = 0x18;          //right   zoom in
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;
						case 0x28: command_byte = 0x19;          //up  zoom in
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;	
						case 0x30: command_byte = 0x1a;          //down  zoom in
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;		
						case 0x2c: command_byte = 0x1b;          //left up  zoom in
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;		
						case 0x34: command_byte = 0x1c;          //left down  zoom in
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;		
						case 0x2a: command_byte = 0x1d;          //right up  zoom in
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;	
						case 0x32: command_byte = 0x1e;          //right down zoom in
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;	
						case 0x44: command_byte = 0x1f;          //left zoom out
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;	
						case 0x42: command_byte=0x20;          //right   zoom out
						        Rocket_fir_data=keyboard_data_buffer[4];
						        Rocket_sec_data=keyboard_data_buffer[5];
								break;
						case 0x48: command_byte=0x21;          //up  zoom out
						        Rocket_fir_data=keyboard_data_buffer[4];
						        Rocket_sec_data=keyboard_data_buffer[5];
								break;	
						case 0x50: command_byte=0x22;          //down  zoom out
						        Rocket_fir_data=keyboard_data_buffer[4];
						        Rocket_sec_data=keyboard_data_buffer[5];
								break;		
						case 0x4c: command_byte=0x23;          //left up  zoom out
						        Rocket_fir_data=keyboard_data_buffer[4];
						        Rocket_sec_data=keyboard_data_buffer[5];
								break;		
						case 0x54: command_byte=0x24;          //left down  zoom out
						        Rocket_fir_data=keyboard_data_buffer[4];
						        Rocket_sec_data=keyboard_data_buffer[5];
								break;		
						case 0x4a: command_byte=0x25;          //right up  zoom out
						        Rocket_fir_data=keyboard_data_buffer[4];
						        Rocket_sec_data=keyboard_data_buffer[5];
								break;	
						case 0x52: command_byte=0x26;          //right down zoom out
						        Rocket_fir_data = keyboard_data_buffer[4];
						        Rocket_sec_data = keyboard_data_buffer[5];
								break;		
						case 0x33:
							break;
						case 0x36:
							break;        
						case 0x09: //开报警输出
							command_byte = 0xff;

						break;
						case 0x0b://清除报警输出

						command_byte = 0xff;
						break;

						case 0x90: 
							
						        command_byte = 0xff;
						        break;
						case 0x91: command_byte = 0xff;
						     	break;	

						case 0x92: command_byte = 0x16;                          //已接收轮巡轨迹数据
							break;

						default:   if (0x00 != keyboard_data_buffer[3])
						            command_byte = 0xff;
							    else command_byte = 0x00;
						        break;			
						}
						break;

					}
				}
		
				}
       }
}


void ReturnPanTiltPos(u8 InquiryStepType)
{
#if 0
	uint return_com;
	uchar return_com2[15];//return_com2[9];//	数据返回：P-P协议，返回的数据少一位。

	SendDataToIPModule(0xff,0,0,1);

#if 1
	delay_3s = 0x00;              
	return_com2[0] = 0x5A;

	if (!InquiryStepType)	
	{
		return_com = panpos;      //pan
		return_com2[2] = return_com;
		return_com >>= 8;
		return_com2[1] = return_com;
		return_com = tiltpos;       //tilt
		return_com2[4] = return_com;
		return_com >>= 8;
		return_com2[3] = return_com;
		return_com2[5] = zoomRate>>8;
		return_com2[6] = zoomRate;
	}
	else 
	{			
		return_com = motorStep2degree(panpos,0);;
		return_com2[2] = return_com;
		return_com >>= 8;
		return_com2[1] = return_com;
		return_com = motorStep2degree(tiltpos,1); 
		return_com2[4] = return_com;
		return_com2[3] = (return_com>>8);

		return_com = zoomRate;
		return_com2[6] = zoomRate;
		return_com >>= 8;		
		return_com2[5] = return_com;
	}

	delayms(40);
	RS485_SendBytes(7,return_com2);
#endif
#endif	
}

void return_cmd_to_camlink(u8 cmd,u8 para)
{
	uchar return_com2[15];//return_com2[9];//	数据返回：P-P协议，返回的数据少一位。

	return_com2[0] = CAMERA_LINK_CMD_HEAD;
	return_com2[1] = 0xb2;
    
	return_com2[2] = cmd;
	return_com2[3] = para;

	return_com2[4] = 0;
	return_com2[5] = 0;
	return_com2[6] = 0;
	return_com2[7] = CAMERA_LINK_CMD_TAIL;

	RS485_SendBytes(8,return_com2);
 	delayms(40);
   
}

void Keyboard_camera_porotocol_2(void)
{
	switch (Protocol_No)
	{
		case PELCO_P: 
		case PELCO_D:
			PELCO_D_P_protocol_analysis_2();
		break;
 		case PROTOCOL_CAM_LINK:
			camera_link_protocol_analysis();
		break;       
		default:
        break;
	}
}

uchar command_analysis(void) //return 1,when received a correct command,or,return 0
{
	uchar value = 0;

	if (1 == delqueue(20,keyboard_data_buffer)) 
	{
		Keyboard_camera_porotocol_2();
		value = 1;
	}

	return(value);
}
