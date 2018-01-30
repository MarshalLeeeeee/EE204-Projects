
#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
#include "uart.h"
#include "hw_ints.h"

#define SYSTICK_FREQUENCY		1000			//1000hz

#define	I2C_FLASHTIME				500				//500mS
#define GPIO_FLASHTIME			300				//300mS
//*****************************************************************************
//
//I2C GPIO chip address and resigster define
//
//*****************************************************************************
#define TCA6424_I2CADDR 					0x22
#define PCA9557_I2CADDR						0x18

#define PCA9557_INPUT							0x00
#define	PCA9557_OUTPUT						0x01
#define PCA9557_POLINVERT					0x02
#define PCA9557_CONFIG						0x03

#define TCA6424_CONFIG_PORT0			0x0c
#define TCA6424_CONFIG_PORT1			0x0d
#define TCA6424_CONFIG_PORT2			0x0e

#define TCA6424_INPUT_PORT0				0x00
#define TCA6424_INPUT_PORT1				0x01
#define TCA6424_INPUT_PORT2				0x02

#define TCA6424_OUTPUT_PORT0			0x04
#define TCA6424_OUTPUT_PORT1			0x05
#define TCA6424_OUTPUT_PORT2			0x06




void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void		S800_I2C0_Init(void);
void 		S800_UART_Init(void);
//systick software counter define
volatile uint16_t systick_10ms_couter,systick_100ms_couter;
volatile uint8_t	systick_10ms_status,systick_100ms_status;
volatile uint16_t	i2c_flash_cnt=0,i2c_flash_cnt1=0,i2c_flash_cnt2=0,gpio_flash_cnt3=0;
volatile uint8_t result,cnt,key_value,gpio_status;
volatile uint8_t rightshift = 0x01;
uint32_t ui32SysClock,ui32IntPriorityGroup,ui32IntPriorityMask;
uint32_t ui32IntPrioritySystick,ui32IntPriorityUart0;
uint32_t flashtime=0,flashtime1=5,flashtime2=5;
uint32_t myMessage_cnt;
uint32_t myMessage[20];

uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
uint8_t uart_receive_char;
uint8_t priority=5,priority1=5,priority2=5,priority3=5;
uint8_t task1=1,task2=1,task3=1;
uint8_t LED1=1,LED2=1;
uint8_t task1_pause=0,task2_pause=0,task3_pause=0;
int minute,second;
 
int main(void)
{
	//volatile uint16_t	i2c_flash_cnt,i2c_flash_cnt1,i2c_flash_cnt2,gpio_flash_cnt;
	//use internal 16M oscillator, PIOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 8000000);		
	//use external 25M oscillator, MOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);		

	//use external 25M oscillator and PLL to 120M
  //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 0000000);;		
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);
	
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);
	SysTickEnable();
	SysTickIntEnable();																		//Enable Systick interrupt
	  

	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_UART_Init();
	
	IntEnable(INT_UART0);	// 
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX, Receive Time Out interrupt
  IntMasterEnable();	
	
	ui32IntPriorityMask				= IntPriorityMaskGet();
	IntPriorityGroupingSet(1);														//Set all priority to pre-emtption priority
	
	IntPrioritySet(INT_UART0,0x00);											//Set INT_UART0 to highest priority
	IntPrioritySet(FAULT_SYSTICK,0xE0);									//Set INT_SYSTICK to lowest priority
	
	ui32IntPriorityGroup			= IntPriorityGroupingGet();

	ui32IntPriorityUart0			= IntPriorityGet(INT_UART0);
	ui32IntPrioritySystick		= IntPriorityGet(FAULT_SYSTICK);
		
	while (1)
	{
		task1_pause=0;
		task2_pause=0;
		task3_pause=0;
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
		{
			if (priority1<=priority3){task3_pause=1;}
			if (priority1<=priority2){task2_pause=1;}
		}
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1) == 0)
		{
			if (priority2<=priority3){task3_pause=1;}
			if (priority2<=priority1){task1_pause=1;}
		}
		if (systick_10ms_status)
		{
			systick_10ms_status		= 0;
				
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x01);	//write port 2
			Delay(10000);
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x02);	//write port 2
			Delay(10000);
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x04);	//write port 2
			Delay(10000);
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x08);	//write port 2
			Delay(10000);
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
			result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x10);	//write port 2
			Delay(10000);
			
			if(task3==1 && task3_pause==0){gpio_flash_cnt3++;}
			
			if (gpio_flash_cnt3>=90)
			{
				gpio_flash_cnt3=0;
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);
				if(rightshift==0x80){rightshift= 0x01;}
				else{rightshift= rightshift<<1;}
			}
			
			/*if (gpio_flash_cnt3	<= 12)
			{
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	
				rightshift= rightshift<<1;
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x01);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x02);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x04);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x08);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x10);	//write port 2
						Delay(10000);
			}
			else if(12 < gpio_flash_cnt3 && gpio_flash_cnt3 <= 24)
			{
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	
				rightshift= rightshift<<1;
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x02);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x04);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x08);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x10);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x20);	//write port 2
						Delay(10000);
			}
			else if(24 < gpio_flash_cnt3 && gpio_flash_cnt3 <= 37)
			{
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	
				rightshift= rightshift<<1;
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x04);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x08);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x10);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x20);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x40);	//write port 2
						Delay(10000);
			}
			else if(37 < gpio_flash_cnt3 && gpio_flash_cnt3 <= 49)
			{
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	
				rightshift= rightshift<<1;
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x08);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x10);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x20);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x40);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x80);	//write port 2
						Delay(10000);
			}
			else if(49 < gpio_flash_cnt3 && gpio_flash_cnt3 <= 62)
			{
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	
				rightshift= rightshift<<1;
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x10);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x20);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x40);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x80);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x01);	//write port 2
						Delay(10000);
			}
			else if(62 < gpio_flash_cnt3 && gpio_flash_cnt3 <= 74)
			{
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	
				rightshift= rightshift<<1;
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x20);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x40);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x80);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x01);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x02);	//write port 2
						Delay(10000);
			}
			else if(74 < gpio_flash_cnt3 && gpio_flash_cnt3 <= 87)
			{
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	
				rightshift= rightshift<<1;
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x40);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x80);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x01);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x02);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x04);	//write port 2
						Delay(10000);
			}
			else if(87 < gpio_flash_cnt3 && gpio_flash_cnt3 <= 99)
			{
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	
				rightshift= rightshift<<1;
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x80);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x01);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	          //write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x02);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x04);	//write port 2
						Delay(10000);
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);	//write port 2
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1 				
						result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x08);	//write port 2
						Delay(10000);
				gpio_flash_cnt3=0;
				rightshift= 0x01;
			}
			else{gpio_flash_cnt3=0;}*/
		}
		if (systick_100ms_status)
		{
			systick_100ms_status	= 0;
			
			if (task1==1)
			{
				if (++i2c_flash_cnt1 >= flashtime1)
				{
					i2c_flash_cnt1=0;
					if (task1_pause==0)
					{
						if (LED1==1)
						{
							GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0);
							LED1=0;
						}
						else if (LED1==0)
						{
							GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
							LED1=1;
						}
					}
				}
			}
			else{GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);}
			
			if (task2==1)
			{
				if (++i2c_flash_cnt2 >= flashtime2)
				{
					i2c_flash_cnt2=0;
					if (task2_pause==0)
					{
						if (LED2==1)
						{
							GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,0);
							LED2=0;
						}
						else if (LED2==0)
						{
							GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
							LED2=1;
						}
					}
				}
			}
			else{GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);}
			
			if (++i2c_flash_cnt		>= 10)
			{
				i2c_flash_cnt=0;
				second++;
				if (second >= 60)
				{
					second=0;
					minute++;
				}
				if (minute >= 60){minute=0;}
				if(task1==1 && task1_pause==0)
				{
					UARTCharPutNonBlocking(UART0_BASE,'T');
					UARTCharPutNonBlocking(UART0_BASE,'A');
					UARTCharPutNonBlocking(UART0_BASE,'S');
					UARTCharPutNonBlocking(UART0_BASE,'K');
					UARTCharPutNonBlocking(UART0_BASE,'1');
					UARTCharPutNonBlocking(UART0_BASE,' ');
					UARTCharPutNonBlocking(UART0_BASE,'i');
					UARTCharPutNonBlocking(UART0_BASE,'s');
					UARTCharPutNonBlocking(UART0_BASE,' ');
					UARTCharPutNonBlocking(UART0_BASE,'r');
					UARTCharPutNonBlocking(UART0_BASE,'u');
					UARTCharPutNonBlocking(UART0_BASE,'n');
					UARTCharPutNonBlocking(UART0_BASE,'n');
					UARTCharPutNonBlocking(UART0_BASE,'i');
					UARTCharPutNonBlocking(UART0_BASE,'n');
					UARTCharPutNonBlocking(UART0_BASE,'g');
					UARTCharPutNonBlocking(UART0_BASE,'!');	
					Delay(10000);
				}
				if(task2==1 && task2_pause==0)
				{
					UARTCharPutNonBlocking(UART0_BASE,'T');
					UARTCharPutNonBlocking(UART0_BASE,'A');
					UARTCharPutNonBlocking(UART0_BASE,'S');
					UARTCharPutNonBlocking(UART0_BASE,'K');
					UARTCharPutNonBlocking(UART0_BASE,'2');
					UARTCharPutNonBlocking(UART0_BASE,' ');
					UARTCharPutNonBlocking(UART0_BASE,'i');
					UARTCharPutNonBlocking(UART0_BASE,'s');
					UARTCharPutNonBlocking(UART0_BASE,' ');
					UARTCharPutNonBlocking(UART0_BASE,'r');
					UARTCharPutNonBlocking(UART0_BASE,'u');
					UARTCharPutNonBlocking(UART0_BASE,'n');
					UARTCharPutNonBlocking(UART0_BASE,'n');
					UARTCharPutNonBlocking(UART0_BASE,'i');
					UARTCharPutNonBlocking(UART0_BASE,'n');
					UARTCharPutNonBlocking(UART0_BASE,'g');
					UARTCharPutNonBlocking(UART0_BASE,'!');	
					Delay(10000);
				}
			}
			/*{
				i2c_flash_cnt				= 0;
				result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[cnt+1]);	//write port 1 				
				result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
		
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	

				cnt++;
				rightshift= rightshift<<1;

				if (cnt		  >= 0x8)
				{
					rightshift= 0x01;
					cnt 			= 0;
				}*/
		}
	}
}


void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void UARTStringPut(uint8_t *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPut(UART0_BASE,*(cMessage++));
}
void UARTStringPutNonBlocking(const char *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPutNonBlocking(UART0_BASE,*(cMessage++));
}

void S800_UART_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);						//Enable PortA
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);												// Set GPIO A0 and A1 as UART pins.
  GPIOPinConfigure(GPIO_PA1_U0TX);    			

  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock,115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
	UARTStringPut((uint8_t *)"\r\nHello, world!\r\n");
}
void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready		
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);			//Set PN0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);			//Set PN1 as Output pin	

	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

void S800_I2C0_Init(void)
{
	uint8_t result;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	

	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8
	
}


uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE)){};
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE)){};
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);

	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	return rop;
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value,rop;
	while(I2CMasterBusy(I2C0_BASE)){};	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
//	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);		
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusBusy(I2C0_BASE));
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	Delay(1);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
		Delay(1);
	return value;
}

//Corresponding to the startup_TM4C129.s vector table systick interrupt program name

void SysTick_Handler(void)
{
	if (systick_100ms_couter	!= 0)
		systick_100ms_couter--;
	else
	{
		systick_100ms_couter	= SYSTICK_FREQUENCY/10;
		systick_100ms_status 	= 1;
	}
	
	if (systick_10ms_couter	!= 0)
		systick_10ms_couter--;
	else
	{
		systick_10ms_couter		= SYSTICK_FREQUENCY/100;
		systick_10ms_status 	= 1;
	}
	
	/*if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)         // Press USER_SW1
	{
		systick_100ms_status	= systick_10ms_status = 0;	
		while(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0){press1=1;}
	}
	else{press1=0;}
	
	if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1) == 0)         // Press USER_SW2
	{
		systick_100ms_status	= systick_10ms_status = 0;
		while(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1) == 0){press2=1;}
	}
	else
	{press2=0;}*/
}

//Corresponding to the startup_TM4C129.s vector table UART0_Handler interrupt program name

void UART0_Handler(void)
{
	int32_t uart0_int_status;
  uart0_int_status 		= UARTIntStatus(UART0_BASE, true);		// Get the interrrupt status.

  UARTIntClear(UART0_BASE, uart0_int_status);								// Clear the asserted interrupts

	for (myMessage_cnt=0;myMessage_cnt<20;myMessage_cnt++){myMessage[myMessage_cnt]=' ';}
	myMessage_cnt=0;
  while(UARTCharsAvail(UART0_BASE))    											// Loop while there are characters in the receive FIFO.
  {
		// Read the next character from the UART and write it back to the UART.
		myMessage[myMessage_cnt]=UARTCharGetNonBlocking(UART0_BASE);
		UARTCharPutNonBlocking(UART0_BASE,myMessage[myMessage_cnt]);
		myMessage_cnt++;
		Delay(1000);
	}
	
	if(myMessage[0]=='T' && myMessage[1]=='A' && myMessage[2]=='S' && myMessage[3]=='K' && myMessage[5]=='+' && myMessage[6]=='S' && myMessage[7]=='T' && myMessage[8]=='A' && myMessage[9]=='R' && myMessage[10]=='T')
	{
		if (myMessage[4]=='1')
		{
			task1=1;
			i2c_flash_cnt1=0;
			LED1=0;
		}
		else if (myMessage[4]=='2')
		{
			task2=1;
			i2c_flash_cnt2=0;
			LED2=0;
		}
		else if (myMessage[4]=='3'){task3=1;}
	}
	else if(myMessage[0]=='T' && myMessage[1]=='A' && myMessage[2]=='S' && myMessage[3]=='K' && myMessage[5]=='+' && myMessage[6]=='S' && myMessage[7]=='T' && myMessage[8]=='O' && myMessage[9]=='P')
	{
		if (myMessage[4]=='1'){task1=0;}
		else if (myMessage[4]=='2'){task2=0;}
		else if (myMessage[4]=='3'){task3=0;}
	}
	else if(myMessage[0]=='T' && myMessage[1]=='A' && myMessage[2]=='S' && myMessage[3]=='K' && myMessage[5]=='+' && myMessage[6]=='F' && myMessage[7]=='L' && myMessage[8]=='A' && myMessage[9]=='S' && myMessage[10]=='H')
	{
		flashtime=5;
		if (myMessage[11]=='2' && myMessage[12]=='0' && myMessage[13]=='0' && myMessage[14]==' '){flashtime=2;}
		else if (myMessage[11]=='5' && myMessage[12]=='0' && myMessage[13]=='0' && myMessage[14]==' '){flashtime=5;}
		else if (myMessage[11]=='1' && myMessage[12]=='0' && myMessage[13]=='0' && myMessage[14]=='0'){flashtime=10;}
		else if (myMessage[11]=='2' && myMessage[12]=='0' && myMessage[13]=='0' && myMessage[14]=='0'){flashtime=20;}
		
		if (myMessage[4]=='1')
		{
			flashtime1=flashtime;
			i2c_flash_cnt1=0;
		}
		else if (myMessage[4]=='2')
		{
			flashtime2=flashtime;
			i2c_flash_cnt2=0;
		}
		flashtime=5;
	}
	else if(myMessage[0]=='T' && myMessage[1]=='A' && myMessage[2]=='S' && myMessage[3]=='K' && myMessage[5]=='+' && myMessage[6]=='P' && myMessage[7]=='R' && myMessage[8]=='I' && myMessage[9]=='O' && myMessage[10]=='R' && myMessage[11]=='I' && myMessage[12]=='T' && myMessage[13]=='Y' && myMessage[14]=='+')
	{
		priority=5;
		if (myMessage[15]=='0'){priority=0;}
		else if (myMessage[15]=='1'){priority=1;}
		else if (myMessage[15]=='2'){priority=2;}
		else if (myMessage[15]=='3'){priority=3;}
		else if (myMessage[15]=='4'){priority=4;}
		else if (myMessage[15]=='5'){priority=5;}
		
		if (myMessage[4]=='1'){priority1=priority;}
		else if (myMessage[4]=='2'){priority2=priority;}
		else if (myMessage[4]=='3'){priority3=priority;}
		priority=5;
	}
}
