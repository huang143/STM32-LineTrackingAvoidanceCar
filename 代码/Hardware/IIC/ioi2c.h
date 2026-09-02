/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */

#ifndef __IOI2C_H
#define __IOI2C_H

#include "sys.h"

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define SDA_IN()  {GPIOB->CRH&=0XFFFFFF0F;GPIOB->CRH|=8<<4;}
#define SDA_OUT() {GPIOB->CRH&=0XFFFFFF0F;GPIOB->CRH|=3<<4;}

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define IIC_SCL    PBout(8) //SCL
#define IIC_SDA    PBout(9) //SDA	 
#define READ_SDA   PBin(9)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void IIC_Init(void);           // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
int IIC_Start(void);					 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void IIC_Stop(void);	  			 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void IIC_Send_Byte(u8 txd);		 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 IIC_Read_Byte(unsigned char ack);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
int IIC_Wait_Ack(void); 			 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void IIC_Ack(void);						 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void IIC_NAck(void);					 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	 
unsigned char I2C_Readkey(unsigned char I2C_Addr);

unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr);
unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data);
u8 IICwriteBytes(u8 dev, u8 reg, u8 length, u8* data);
u8 IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data);
u8 IICwriteBit(u8 dev,u8 reg,u8 bitNum,u8 data);
u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data);

int i2cWrite(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);
int i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);

#endif

//------------------End of File----------------------------
