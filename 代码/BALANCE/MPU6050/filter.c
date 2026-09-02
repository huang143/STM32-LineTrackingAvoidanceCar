/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */

#include "filter.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
float dt=0.005;		  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
float Kalman_Filter_x(float Accel,float Gyro)		
{
	static float angle_dot;
	static float angle;
	float Q_angle=0.001; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	float Q_gyro=0.003;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	float R_angle=0.5;		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	char  C_0 = 1;
	static float Q_bias, Angle_err;
	static float PCt_0, PCt_1, E;
	static float K_0, K_1, t_0, t_1;
	static float Pdot[4] ={0,0,0,0};
	static float PP[2][2] = { { 1, 0 },{ 0, 1 } };
	angle+=(Gyro - Q_bias) * dt; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	Pdot[0]=Q_angle - PP[0][1] - PP[1][0]; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

	Pdot[1]=-PP[1][1];
	Pdot[2]=-PP[1][1];
	Pdot[3]=Q_gyro;
	PP[0][0] += Pdot[0] * dt;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PP[0][1] += Pdot[1] * dt;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PP[1][0] += Pdot[2] * dt;
	PP[1][1] += Pdot[3] * dt;
		
	Angle_err = Accel - angle;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	
	PCt_0 = C_0 * PP[0][0];
	PCt_1 = C_0 * PP[1][0];
	
	E = R_angle + C_0 * PCt_0;
	
	K_0 = PCt_0 / E;
	K_1 = PCt_1 / E;
	
	t_0 = PCt_0;
	t_1 = C_0 * PP[0][1];

	PP[0][0] -= K_0 * t_0;		 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PP[0][1] -= K_0 * t_1;
	PP[1][0] -= K_1 * t_0;
	PP[1][1] -= K_1 * t_1;
		
	angle	+= K_0 * Angle_err;	 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	Q_bias	+= K_1 * Angle_err;	 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	angle_dot   = Gyro - Q_bias;	 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	return angle;
}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
float Complementary_Filter_x(float angle_m, float gyro_m)
{
	 static float angle;
	 float K1 =0.02; 
   angle = K1 * angle_m+ (1-K1) * (angle + gyro_m * dt);
	 return angle;
}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
float Kalman_Filter_y(float Accel,float Gyro)		
{
	static float angle_dot;
	static float angle;
	float Q_angle=0.001; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	float Q_gyro=0.003;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	float R_angle=0.5;		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	char  C_0 = 1;
	static float Q_bias, Angle_err;
	static float PCt_0, PCt_1, E;
	static float K_0, K_1, t_0, t_1;
	static float Pdot[4] ={0,0,0,0};
	static float PP[2][2] = { { 1, 0 },{ 0, 1 } };
	angle+=(Gyro - Q_bias) * dt; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	Pdot[0]=Q_angle - PP[0][1] - PP[1][0]; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	Pdot[1]=-PP[1][1];
	Pdot[2]=-PP[1][1];
	Pdot[3]=Q_gyro;
	PP[0][0] += Pdot[0] * dt;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PP[0][1] += Pdot[1] * dt;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PP[1][0] += Pdot[2] * dt;
	PP[1][1] += Pdot[3] * dt;
	Angle_err = Accel - angle;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	
	PCt_0 = C_0 * PP[0][0];
	PCt_1 = C_0 * PP[1][0];
	
	E = R_angle + C_0 * PCt_0;
	
	K_0 = PCt_0 / E;
	K_1 = PCt_1 / E;
	
	t_0 = PCt_0;
	t_1 = C_0 * PP[0][1];

	PP[0][0] -= K_0 * t_0;		 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PP[0][1] -= K_0 * t_1;
	PP[1][0] -= K_1 * t_0;
	PP[1][1] -= K_1 * t_1;
		
	angle	+= K_0 * Angle_err;	   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	Q_bias	+= K_1 * Angle_err;	 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	angle_dot   = Gyro - Q_bias;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	return angle;
}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
float Complementary_Filter_y(float angle_m, float gyro_m)
{
	 static float angle;
	 float K1 =0.02; 
   angle = K1 * angle_m+ (1-K1) * (angle + gyro_m * dt);
	 return angle;
}


