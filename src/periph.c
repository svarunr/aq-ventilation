/*
 * peripherals.c
 *
 *  Created on: Mar. 15, 2020
 *      Author: varunrathod
 */
#include <periph.h>
// Configure the GPIO.
void GPIO_Config(void) {
	// Turn on the GPIO module.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK);
	// Reset the GPIO module.
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK);
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK);
	// Turn LEDs ON.
	GPIO->CLR[0] = (1UL<<LED2);
	// Set the direction.
	GPIO->DIRSET[0] = (1UL<<LED2);
	GPIO->DIRCLR[0] = (1UL<<BUTTON) | (1UL<<SENSEA) | (1UL<<SENSEB);
	// Turn on the interrupt for the encoder.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);
	SYSCON->PINTSEL[0] = (SENSEA);
	SYSCON->PINTSEL[1] = (BUTTON);
	PINT->ISEL  = 0x00;
	PINT->CIENF = 0x01;
	PINT->CIENR = 0x02;
	PINT->SIENR = 0x01;
	PINT->SIENF = 0x02;
	PINT->IST   = 0xFF;
	// Enable interrupts.
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
}
