/**
* EEPROM Table by samit
**/

#include "ConfigTable.h"
#include "stmflash.h"
#include "Control.h"
#include "SysConfig.h"
#include "NRF24L01.h"
#include "CommUAV.h"
//
#define TABLE_ADDRESS (STM32_FLASH_BASE+STM32_FLASH_OFFEST+0)
//用来存放EEPROM列表上的存放的参数变量的信息
config_table_t table;				     //tobe improved: config mean in const / eeprom.
//请求保存参数到EEPROM的信号量
uint8_t gParamsSaveEEPROMRequset=0;

#define EEPROM_DEFAULT_VERSION 1





extern uint16_t Throttle_Calibra;
extern uint16_t Pitch_Calibra;
extern uint16_t Roll_Calibra;
extern uint16_t Yaw_Calibra ;//摇杆校准值



//table defalat . if
void TableResetDefault(void)
{
    STMFLASH_Write(TABLE_ADDRESS,(uint16_t *)(&(table.version)),2);
}
//load params from EEPROM
void TableReadEEPROM(void)
{
    uint8_t paramNums=sizeof(table)/sizeof(uint16_t);
    STMFLASH_Read(TABLE_ADDRESS,(uint16_t *)(&table),paramNums * 2);
}

//write params to EEPROM
void TableWriteEEPROM(void)
{
    uint8_t paramNums=sizeof(table)/sizeof(uint16_t);

    STMFLASH_Write(TABLE_ADDRESS,(uint16_t *)(&table),paramNums * 2);
}

void TableToParam(void)
{

}

void ParamToTable(void)
{

}

void LoadParamsFromEEPROM(void)
{

}

void SaveParamsToEEPROM(void)
{
    ParamToTable();
    TableWriteEEPROM();
}

//all default value
void ParamSetDefault(void)
{

}
