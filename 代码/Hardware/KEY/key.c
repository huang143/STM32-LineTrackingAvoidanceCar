#include "key.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
u8 click_N_Double (u8 time)
{
		static	u8 flag_key,count_key,double_key;	
		static	u16 count_single,Forever_count;
	  if(KEY==0)  Forever_count++;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
     else        Forever_count=0;
		if(0==KEY&&0==flag_key)		flag_key=1;	
	  if(0==count_key)
		{
				if(flag_key==1) 
				{
					double_key++;
					count_key=1;	
				}
				if(double_key==2) 
				{
					double_key=0;
					count_single=0;
					return 2;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
				}
		}
		if(1==KEY)			flag_key=0,count_key=0;
		
		if(1==double_key)
		{
			count_single++;
			if(count_single>time&&Forever_count<time)
			{
			double_key=0;
			count_single=0;	
			return 1;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			}
			if(Forever_count>time)
			{
			double_key=0;
			count_single=0;	
			}
		}	
		return 0;
}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
u8 click(void)
{
	static u8 flag_key=1;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	if(flag_key&&KEY==0)
	{
	flag_key=0;
	return 1;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	}
	else if(1==KEY)			flag_key=1;
	return 0;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
u8 Long_Press(void)
{
		static u16 Long_Press_count,Long_Press;
	    if(Long_Press==0&&KEY==0)  Long_Press_count++;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
        else                       Long_Press_count=0; 
		if(Long_Press_count>200)		
		{
			Long_Press=1;
			Long_Press_count=0;
			return 1;
		}			
			 if(Long_Press==1)     // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			{
				  Long_Press=0;
			}
			return 0;
}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
u8  select(void)
{
	  u8 i;
	  static u8 flag=1;
	  oled_show_once();  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	  i=click_N_Double(50);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	  if(i==1)
	  { 
		 Mode++;
		 if(Mode==0) BLUETOOTH_Mode=1,CCD_Line_Patrol_Mode=0,ELE_Line_Patrol_Mode=0,PS2_Mode=0;
		 if(Mode==1) BLUETOOTH_Mode=0,CCD_Line_Patrol_Mode=0,ELE_Line_Patrol_Mode=0,PS2_Mode=1;	
         if(Mode==2) CCD_Line_Patrol_Mode=1,BLUETOOTH_Mode=0,ELE_Line_Patrol_Mode=0,PS2_Mode=0;
         if(Mode==3) ELE_Line_Patrol_Mode=1,BLUETOOTH_Mode=0,CCD_Line_Patrol_Mode=0,PS2_Mode=0;
         if(Mode==4) Mode=0,BLUETOOTH_Mode=1,CCD_Line_Patrol_Mode=0,ELE_Line_Patrol_Mode=0,PS2_Mode=0;	
         
	  }
	  if(i==2)OLED_Clear(),flag=0;  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	  return flag;	  
}
