
extern uchar command_analysis(void); 
extern 	void delay_X1ms(uint m);
void camera_power_on_off(u8 mode);
void camera_sensor_format_set_with_long_key(void);
void camera_ntsc_pal_switch(u8 mode);
void camera_cvbs_switch(u8 mode);
void camera_hd_switch(u8 mode);
void iris_ex_pin_set(u8 mode);

extern void my_proto_call_preset(u8 cno,u8 val,u8 addr);
extern void my_proto_set_preset(u8 cno,u8 val,u8 addr);
extern void sys_expand_para_initial(void);

