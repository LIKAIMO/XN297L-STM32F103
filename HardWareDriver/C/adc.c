#include "adc.h"
#include "delay.h"
#include "config.h"
#include "stm32f10x_adc.h"


//初始化ADC
//这里我们仅以规则通道为例
//开启ADC1的通道0 1 2 3 对应 PA0 PA1 PA2	PA3
//Ch0---->PA0
//Ch1---->PA1
//Ch2---->PA2
//Ch3---->PA3
void  Adc_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_ADC1, ENABLE );
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);//最大不能超过14M

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);
    delay_us(100);//GD32移植需要在使能之后立即加100us延时
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/*
获取通道ch的转换值，取times次,然后平均
ch:通道编号
times:获取次数
返回值:通道ch的times次转换结果平均值
*/
u16
Get_Adc_Average(u8 ch,u8 times)
{
    u8 i = 0;
    u32 temp_val = 0;
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);//ADC1,ADC通道ch,规则采样顺序值序列为1,采样时间为239.5周期
    for(i=0; i<times; i++) {
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);//软件启动转换启动功能
        while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));//等待转换结束
        ADC_ClearFlag(ADC1,ADC_FLAG_EOC);//清除转换标志位，GD32必须手动清除
        temp_val += ADC_GetConversionValue(ADC1);
    }
    return temp_val/times;//返回平均值
}

//得到ADC采样内部温度传感器的温度值
//返回值3位温度值 XXX*0.1C
int Get_Temp(void)
{
    u16 temp_val=0;
    float temperate;
    temp_val/= Get_Adc_Average(ADC_CH_TEMP,20);
    temperate=(float)temp_val*(3.3/4096);//得到温度传感器的电压值
    temperate=(1.43-temperate)/0.0043+25;//计算出当前温度值
    temperate*=10;//扩大十倍,使用小数点后一位
    return (int)temperate;
}

void powerOn(void)
{
    static uint8_t isPower = 0;
    static uint8_t int80Cnt = 0;
    static uint8_t softSwitchFlag = 0;

    if(isPower == 0)
    {
        if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6) == 0)
        {
            int80Cnt++;
        }
        else
        {
            int80Cnt = 0;
        }
        if(int80Cnt > 150)
        {
            int80Cnt = 0;
#ifndef NEWC03
            GPIO_ResetBits(GPIOA, GPIO_Pin_14);
            GPIO_SetBits(GPIOB,GPIO_Pin_7);
            delay_ms(1000);
            GPIO_SetBits(GPIOA, GPIO_Pin_14);
#else
            GPIO_ResetBits(GPIOB, GPIO_Pin_10);
            GPIO_SetBits(GPIOB,GPIO_Pin_7);
            delay_ms(1000);
            GPIO_SetBits(GPIOB, GPIO_Pin_10);
#endif
            isPower = 1;
        }
    }
    else
    {
        if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6) != 0)
        {
            softSwitchFlag = 1;
            int80Cnt = 0;
        }
        else if(softSwitchFlag)
        {
            int80Cnt++;
            if(int80Cnt > 80)
            {
#ifndef NEWC03
                GPIO_ResetBits(GPIOA, GPIO_Pin_14);
                delay_ms(300);
                GPIO_ResetBits(GPIOB, GPIO_Pin_7);
                GPIO_SetBits(GPIOA, GPIO_Pin_14);
#else
                GPIO_ResetBits(GPIOB, GPIO_Pin_10);
                delay_ms(300);
                GPIO_ResetBits(GPIOB, GPIO_Pin_7);
                GPIO_SetBits(GPIOB, GPIO_Pin_10);
#endif
                while(1);
            }
        }
    }
}

void lowVoltageBeep(void)
{
    //static char x = 0;
    const float batteryRef = 3.26;
    int batteryAD = Get_Adc_Average(8,5);//获取检测电压的AD值
    float	batteryVal = batteryAD/4096.0*batteryRef*2;
    /*	x++;
    	while(x == 10){
    			printf("V:%f\r",batteryVal);
    			x = 0;
    	}
    */
    if (batteryVal < 3.5)
    {
        static int beeperTime = 0;
        static uint8_t beeperSta = 0;
        beeperTime++;
        if(beeperTime > 10)
        {
            beeperSta = !beeperSta;
            beeperTime = 0;
        }
        if (beeperSta == 1)
        {
#ifndef NEWC03
            GPIO_ResetBits(GPIOA, GPIO_Pin_14);
#else
            GPIO_ResetBits(GPIOB, GPIO_Pin_10);
#endif
        }
        else
        {
#ifndef NEWC03
            GPIO_SetBits(GPIOA, GPIO_Pin_14);
#else
            GPIO_SetBits(GPIOB, GPIO_Pin_10);
#endif
        }
    }
}









