/*    
      ____                      _____                  +---+
     / ___\                     / __ \                 | R |
    / /                        / /_/ /                 +---+
   / /   ________  ____  ___  / ____/___  ____  __   __
  / /  / ___/ __ `/_  / / _ \/ /   / __ \/ _  \/ /  / /
 / /__/ /  / /_/ / / /_/  __/ /   / /_/ / / / / /__/ /
 \___/_/   \__,_/ /___/\___/_/    \___ /_/ /_/____  /
                                                 / /
                                            ____/ /
                                           /_____/
*/
/* Control.c file
编写者：小马  (Camel)
作者E-mail：375836945@qq.com
编译环境：MDK-Lite  Version: 4.23
初版时间: 2014-01-28
功能：
------------------------------------
*/

#include "config.h"
#include "adc.h"
#include "control.h"
#include "math.h"

#define CONTAIN(a,b) ((a)<(b)?(a):(b))
const uint8_t DEADBAND = 100;
void setBand(uint16_t *value, uint16_t limit)
{
	const int range[2] = {1500-DEADBAND,1500+DEADBAND};
	const int controlRange = 2000-(1500+DEADBAND);
	const float proportion = (limit-range[1])/controlRange; // limit > 1500+DEADBAND
	if(*value > range[1])
	{
		*value += (*value-range[1])*proportion;
	}
	else if(*value < range[0])
	{
		*value += (*value-range[0])*proportion;
	}
}

/*
摇杆数据采集
输入参数为美国手和日本手
*/
#ifdef AMERICAN_RC_MODE
	#define THROTTLE_CHANNEL 3
	#define PITCH_CHANNEL 1
#else
	#define THROTTLE_CHANNEL 1
	#define PITCH_CHANNEL 3
#endif
#define ROLL_CHANNEL 0
#define YAW_CHANNEL 2

void LoadRCdata(void)
{
	remoteData.motor[YAW]=(((1000+(uint16_t)(Get_Adc_Average(YAW_CHANNEL,15)*1000>>12))-1500)>>1)+1500;
	remoteData.motor[ROLL]=(((1000+(uint16_t)(Get_Adc_Average(ROLL_CHANNEL,15)*1000>>12))-1500)>>1)+1500;	
#ifdef AMERICAN_RC_MODE
	remoteData.motor[THROTTLE]=(2000-(uint16_t)(Get_Adc_Average(THROTTLE_CHANNEL,5)*1000>>12));	
	remoteData.motor[PITCH]=(((1000+(uint16_t)(Get_Adc_Average(PITCH_CHANNEL,15)*1000>>12))-1500)>>1)+1500;
#else
	remoteData.motor[THROTTLE]=1000+(uint16_t)(Get_Adc_Average(THROTTLE_CHANNEL,15)*1000>>12);	
	remoteData.motor[PITCH]=(2000-(uint16_t)(Get_Adc_Average(PITCH_CHANNEL,15)*1000>>12));
#endif
		
	remoteData.motor[THROTTLE]=(fabs(remoteData.motor[THROTTLE]-1500)>10)?remoteData.motor[THROTTLE]:1500;
	remoteData.motor[THROTTLE]=CONTAIN(2000,remoteData.motor[THROTTLE]);
	remoteData.motor[THROTTLE]=-CONTAIN(-remoteData.motor[THROTTLE],-1000);
	remoteData.motor[YAW]=(fabs(remoteData.motor[YAW]-1500)>30)?remoteData.motor[YAW]:1500;
	remoteData.motor[YAW]=CONTAIN(2000,remoteData.motor[YAW]);
	remoteData.motor[YAW]=-CONTAIN(-remoteData.motor[YAW],-1000);
	remoteData.motor[ROLL]=(fabs(remoteData.motor[ROLL]-1500)>30)?remoteData.motor[ROLL]:1500;
	remoteData.motor[ROLL]=CONTAIN(1750,remoteData.motor[ROLL]);
	remoteData.motor[ROLL]=-CONTAIN(-remoteData.motor[ROLL],-1250);
	remoteData.motor[PITCH]=(fabs(remoteData.motor[PITCH]-1500)>30)?remoteData.motor[PITCH]:1500;
	remoteData.motor[PITCH]=CONTAIN(1750,remoteData.motor[PITCH]);
	remoteData.motor[PITCH]=-CONTAIN(-remoteData.motor[PITCH],-1250);
	
}


