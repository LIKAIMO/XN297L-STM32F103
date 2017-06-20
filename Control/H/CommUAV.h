#ifndef COMM_UAV
#define COMM_UAV

#include <string.h>
#include "NRF24L01.h"

//---------UAV control cmd

typedef enum _onlineCmd
{
	ONLINE_ARM = 1,
	ONLINE_DISARM,
	ONLINE_ALTHOLD,
	ONLINE_NOTALTHOLD,
	ONLINE_CALIBRATION,
	
	ONLINE_LED,
	ONLINE_BEEP,
	ONLINE_COLOR,
	
	ONLINE_MOTOR,
	ONLINE_STICK,
}onlineCmd_e;

typedef enum _CMD
{
	ARM = 1 << 0,
	FREEHEAD = 1 << 1,
	ALTHOLD = 1 << 2,
	CALIBRATION = 1 << 3,
	NEWADDRESS = 1 << 4,
	ONLINE = 1 << 5,
	OFFLINE = 1 << 6,
	MOTOR = 1 << 7,
}CMD;

enum
{	
	ROLL,
	PITCH,
	YAW,
	THROTTLE,
};

typedef struct _remoteMsg
{
	u8 checkCode[4];
	u8 length;
	u16 cmd;
	u16 motor[4];
	u8 led;
	u8 color;
	u8 beep;
	u8 key;
}remoteMsg;

typedef struct _flightMsg
{
	u8 checkCode[4];
	u8 version;
	u16 status;
	u8 key;
	u16 voltage;
	int16_t rollAngle;
	int16_t pitchAngle;
	int16_t yawAngle;	
	
	
}flightMsg;

extern flightMsg flightData;

extern remoteMsg remoteData;
uint8_t sendF3Data(void);
void sendScratchData(void);
void cmdReduce(void);
	

#endif
