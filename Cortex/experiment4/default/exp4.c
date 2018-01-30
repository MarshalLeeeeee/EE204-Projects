#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_types.h"
#include "pin_map.h"
#include "sysctl.h"


#define   FASTFLASHTIME			(uint32_t)500000
#define   SLOWFLASHTIME			(uint32_t)4000000

uint32_t delay_time,key_value,LED_flag;


void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);


int main(void)
{
	S800_GPIO_Init();
	while(1)
    {
		delay_time = FASTFLASHTIME;
		key_value = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	;				//read the PJ0 key value

		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 0)						//USR_SW1-PJ0 pressed
		{
			if (++LED_flag==5){LED_flag=1;}
			while (1)
			{
				Delay(1000000);
				if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 1) break;
			}
		}
		if (LED_flag==1)
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);						  // Turn on the LED.
			Delay(delay_time);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
			Delay(delay_time);
    }
		else if (LED_flag==2)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
		else if (LED_flag==3)
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);						  // Turn on the LED.
			Delay(delay_time);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);							// Turn off the LED.
			Delay(delay_time);
    }
		else if (LED_flag==4)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);							// Turn off the LED.
	}
}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			    //Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			    //Wait for the GPIO moduleJ ready

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PF0,PF1 as Output pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);              //Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}


