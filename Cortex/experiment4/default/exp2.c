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

uint32_t key_value1,key_value2;


void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);


int main(void)
{
	S800_GPIO_Init();
	while(1)
  {
		key_value1 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0);				//read the PJ0 key value
	  key_value2 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1);				//read the PJ1 key value

		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 0)						//USR_SW1-PJ0 pressed
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);							// Turn on the LED.
		else
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.

		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)	== 0)						//USR_SW1-PJ1 pressed
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);							// Turn on the LED.
		else
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);							// Turn off the LED.
   }
}


void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void S800_GPIO_Init(void)
{
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);//Set PF0,PF4 as Output pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}


