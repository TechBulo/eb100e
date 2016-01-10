#ifndef _ROTOCOL_H_
#define _ROTOCOL_H_

extern void System_data_init(void);
extern void ReturnPanTiltPos(u8 InquiryStepType);
extern void Keyboard_camera_porotocol_2(void);
extern uchar command_analysis(void);


#define CAMERA_LINK_CMD_HEAD    0xb1
#define CAMERA_LINK_CMD_TAIL    0xee



#endif  /* _ROTOCOL_H_ */

