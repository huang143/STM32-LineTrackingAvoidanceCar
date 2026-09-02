#include "led.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */

/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void Led_Flash(u16 time)
{
	  static int temp;
	  if  (0==time) LED=0;                               // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	  else if(++temp==time)	LED=~LED,temp=0;
}
