  /*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */

#ifndef __DATA_PRTOCOL_H
#define __DATA_PRTOCOL_H
 
 
extern unsigned char DataScope_OutPut_Buffer[42];	   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。


void DataScope_Get_Channel_Data(float Data,unsigned char Channel);    // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

unsigned char DataScope_Data_Generate(unsigned char Channel_Number);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
 
 
#endif 



