/****************************************************************


*****************************************************************/
//#include "stm32f10x_type.h"
#include "stm32f10x.h"              /* STM32F10x Library Definitions      */


#ifndef _AT45SPIFLASH_H
#define _AT45SPIFLASH_H 1

/* Select SPI FLASH: ChipSelect pin low  */
//#define SPI_FLASH_CS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_1)
#define SPI_FLASH_CS_LOW()     GPIO_ResetBits(GPIOC, GPIO_Pin_4)
/* Deselect SPI FLASH: ChipSelect pin high */
#define SPI_FLASH_CS_HIGH()    GPIO_SetBits(GPIOC, GPIO_Pin_4)

#define BUFFER_1_WRITE 0x84					// D¡ä¨¨?¦Ì¨²¨°??o3???
#define BUFFER_2_WRITE 0x87					// D¡ä¨¨?¦Ì¨²?t?o3???
#define BUFFER_1_READ 0xD4					// ?¨¢¨¨?¦Ì¨²¨°??o3???
#define BUFFER_2_READ 0xD6					// ?¨¢¨¨?¦Ì¨²?t?o3???
#define B1_TO_MM_PAGE_PROG_WITH_ERASE 0x83	// ??¦Ì¨²¨°??o3???¦Ì?¨ºy?YD¡ä¨¨??¡Â¡ä?¡ä¡é?¡Â¡ê¡§2¨¢3y?¡ê¨º?¡ê?
#define B2_TO_MM_PAGE_PROG_WITH_ERASE 0x86	// ??¦Ì¨²?t?o3???¦Ì?¨ºy?YD¡ä¨¨??¡Â¡ä?¡ä¡é?¡Â¡ê¡§2¨¢3y?¡ê¨º?¡ê?
#define MM_PAGE_TO_B1_XFER 0x53				// ???¡Â¡ä?¡ä¡é?¡Â¦Ì????¡§¨°3¨ºy?Y?¨®??¦Ì?¦Ì¨²¨°??o3???
#define MM_PAGE_TO_B2_XFER 0x55				// ???¡Â¡ä?¡ä¡é?¡Â¦Ì????¡§¨°3¨ºy?Y?¨®??¦Ì?¦Ì¨²?t?o3???
#define PAGE_ERASE 0x81						// ¨°3¨¦?3y¡ê¡§??¨°3512/528¡Á??¨²¡ê?
#define SECTOR_ERASE 0x7C					// ¨¦¨¨??2¨¢3y¡ê¡§??¨¦¨¨??128K¡Á??¨²¡ê?
#define READ_STATE_REGISTER 0x05//0xD7			// ?¨¢¨¨?¡Á¡ä¨¬???¡ä??¡Â
#define READ_SEQUENCE 0xE8

extern unsigned char DF_buffer[528];

void spi_flash_init(void);
u32 SPI_FLASH_ReadID(void);
u8 SPI_FLASH_SendByte(u8 byte);
u8 SPI_FLASH_ReadByte(u8 Dummy_Byte);
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_StartReadSequence(u32 ReadAddr);
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);


void write_SPI				(unsigned char data);
void DF_format				(void);
void DF_page_earse			(unsigned int page);

void DF_write_page			(u8* pBuffer,u16 page);
void DF_read_page			(u8* pBuffer,u16 page);
void DF_mm_read_page		(unsigned int page);
unsigned char DF_read_reg	(void);
void DF_wait_busy			(void);
void DF_mm_to_buf			(unsigned char buffer,unsigned int page);

u8 DF_read_buf(u8 buffer,u16 start_address,u16 length,u8* pBuffer);
u8 DF_write_buf(u8 buffer,u16 start_address,u16 length,u8* pBuffer);

void DF_buf_to_mm			(unsigned char buffer,unsigned int page);

extern u8 eeprom_byte_read(u32 address);
extern void eeprom_byte_write(u32 address, u8 data);

#endif

