#include "UART1.h"
#include "stdio.h"
#include "Control.h"
#include "stm32f10x_it.h"
#include "math.h"
#include "led.h"
#include "ConfigTable.h"
#include "config.h"
#include "CommUAV.h"

//uart reicer flag
#define b_uart_head  0x80
#define b_rx_over    0x40
//flightMsg flightData;
u8 rxbuf[128] = {0};
u8 *pRX = rxbuf;
const u8 *PRX_END = rxbuf+100;
uint8_t modifyOnceFlag = 1;


#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;
};
/* FILE is typedef’ d in stdio.h. */
FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
_sys_exit(int x)
{
    x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
    USART1->SR;  //USART_GetFlagStatus(USART1, USART_FLAG_TC) 解决第一个字符发送失败的问题
    //一个一个发送字符
    USART_SendData(USART1, (unsigned char) ch);
    //等待发送完成
    while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);

    return(ch);
}

static void checkData(void);
static void reduceOnlineCmd(uint8_t cmd);

/**************************实现函数********************************************
*函数原型:		void U1NVIC_Configuration(void)
*功　　能:		串口1中断配置
输入参数：无
输出参数：没有
*******************************************************************************/
void UART1NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}



/**************************实现函数********************************************
*函数原型:		void Initial_UART1(u32 baudrate)
*功　　能:		初始化UART1
输入参数：u32 baudrate   设置RS232串口的波特率
输出参数：没有
*******************************************************************************/
void UART1_init(u32 bound)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    //USART1 Tx(PA.09)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //USART1 Rx(PA.10)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //USART1配置
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}



//------------------------------------------------------
void USART1_IRQHandler(void)
{

    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        //硬件会自动清除, 为防止意料之外的错误,	软件清除接收中断标志
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);

        *pRX = USART_ReceiveData(USART1);
        if(pRX++ > PRX_END)
        {
            pRX = rxbuf;
        }
		printf("this:%d",rxbuf[0]);
    }
}

void reduceOnlineData(void)
{
	//printf("%d\r\n",rxbuf[0]);
	if(rxbuf[0] != 0)
	{
		
		if(rxbuf[0] != '$')
		{
			rxbuf[0] = 0;
		} else if(rxbuf[1] != 'M')
		{
			rxbuf[0] = 0;
			rxbuf[1] = 0;
		} else if(rxbuf[2] != '<')
		{
			rxbuf[0] = 0;
			rxbuf[1] = 0;
			rxbuf[2] = 0;
		}
		else
		{
			checkData();
			rxbuf[0] = 0;
			rxbuf[1] = 0;
			rxbuf[2] = 0;
			printf("{\"roll\":\"%d\",\"pitch\":\"%d\",\"yaw\":\"%d\",\"key\":\"%d\",\"voltage\":\"%d\",\"rollStick\":\"%d\",\"pitchStick\":\"%d\",\"thrStick\":\"%d\",\"yawStick\":\"%d\",\"RVFV\":\"%d%d\"}",
				   flightData.rollAngle,flightData.pitchAngle,flightData.yawAngle,remoteData.key,flightData.voltage,(remoteData.motor[ROLL]-1000)/4,(remoteData.motor[PITCH]-1000)/4,(remoteData.motor[THROTTLE]-1000)/4,
				   (remoteData.motor[YAW]-1000)/4,VERSION_RC,flightData.version);
		}
		
		pRX = rxbuf;
		 
	}
}
void reduceOnlineCmd(uint8_t cmd)
{
    switch(cmd)
    {
    case ONLINE_MOTOR:
    case ONLINE_STICK:
        remoteData.motor[0] = 1000+(rxbuf[4]<<2);
        remoteData.motor[1] = 1000+(rxbuf[5]<<2);
        remoteData.motor[2] = 1000+(rxbuf[6]<<2);
        remoteData.motor[3] = 1000+(rxbuf[7]<<2);
        if(cmd == ONLINE_MOTOR)
        {
            remoteData.cmd |= MOTOR;

        }
        else
        {
            remoteData.cmd &= (~MOTOR);
        }
        break;
    case ONLINE_ARM:

        if((remoteData.cmd & ARM) == 0)
        {
            remoteData.cmd |= ARM;
            remoteData.cmd &= (~MOTOR);
            remoteData.motor[0] = 1500;
            remoteData.motor[1] = 1500;
            remoteData.motor[2] = 1500;
            remoteData.motor[3] = 1000;
        }
        break;
    case ONLINE_DISARM:
        if(remoteData.cmd & ARM)
        {
            remoteData.cmd &= (~ARM);
        }
        remoteData.motor[0] = 1000;
        remoteData.motor[1] = 1000;
        remoteData.motor[2] = 1000;
        remoteData.motor[3] = 1000;
        break;
    case ONLINE_CALIBRATION:
        remoteData.cmd |= CALIBRATION;
        break;
    case ONLINE_LED:
        remoteData.led = rxbuf[8];
        break;
    case ONLINE_COLOR:
        remoteData.color = rxbuf[9];
        break;
    case ONLINE_ALTHOLD:
        if((remoteData.cmd & ALTHOLD) == 0)
        {
            remoteData.cmd |= ALTHOLD;
        }
        break;
    case ONLINE_NOTALTHOLD:
        if(remoteData.cmd & ALTHOLD)
        {
            remoteData.cmd &= (~ALTHOLD);
        }
        break;
    case ONLINE_BEEP:
        remoteData.beep = rxbuf[10];
        break;
    default:
        break;
    }
}
void checkData(void)
{
    uint8_t checkNum = 0;
    uint8_t i,n = 11;
    for(i = 3; i < n; i++)
    {
        checkNum ^= rxbuf[i];
    }

    if(checkNum == rxbuf[n])
    {
        reduceOnlineCmd(rxbuf[3]);
    }
}

void modifyAddress(void)
{

}


