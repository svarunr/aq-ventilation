// File test.c for 3215 labG on MRT.
#include <lpc802_i2c.h>
#include <stdio.h>
#include <math.h>
#include <peripherals.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC802.h"
#include "fsl_debug_console.h"

// Custom header files created by shipping out functions to other source files.
#include "lm75.h"
#include "handle.h"
#include "periph.h"
#include "ssd1306.h"
#include "ccs811.h"

#define MRT_REPEAT (0)
#define MRT_ONESHOT (1)
#define MRT_ONESHOT_BusStall (2)
#define MRT_GFLAG0 (0)
#define MRT_GFLAG1 (1)
#define IVALUE_CH0 (24000000)
#define IVALUE_CH1 (27000000) // CH[0] + 250 ms at 12 MHz FRO.
#define INIT_COUNT_CTIMER (120000)
#define INIT_PWM_DUTY_FACTOR (1)
#define setpoint (23)

sensor t_sensor;

float   ep,
	ei = 0,
	ed,
	pid_out,
	placeh;
uint8_t Kp = 100,
	Ki = 2,
	Kd = 1,
	counter = 0;

motor_mode LO = LOW;
motor_mode HI = HIGH;

void MRT0_IRQHandler(void) {

	// Check MRT trigger channel.
	if(MRT0->IRQ_FLAG&(1<<MRT_GFLAG0)){
		// REMEMBER THAT CLEARING THE FLAG IS IMPORTANT.
		MRT0->CHANNEL[0].STAT = MRT_CHANNEL_STAT_INTFLAG_MASK;	
		
		// This is the part where I compute the PID output of
		// the readings.
		lm75_read(&t_sensor.dat);
		
		if ((t_sensor.dat > 24) & ~(t_sensor.pid)) {
			// Initial error calculation.
			t_sensor.err = (uint8_t) (t_sensor.dat - setpoint);
			
			// Enable the MRT Channel 1 for PID calculations (250 ms + CH[0] interval).
			MRT0->CHANNEL[1].CTRL = (MRT_REPEAT << MRT_CHANNEL_CTRL_MODE_SHIFT |
									 MRT_CHANNEL_CTRL_INTEN_MASK);
			MRT0->CHANNEL[1].INTVAL = IVALUE_CH1 &~ (MRT_CHANNEL_INTVAL_LOAD_MASK);
			
			// Enable the CTimer counter.
			CTIMER0->MR[0]  = (int) ((INIT_PWM_DUTY_FACTOR)*INIT_COUNT_CTIMER);
			CTIMER0->TCR   |= (CTIMER_TCR_CEN_MASK);
			
			// Set flag that PID is being computed, so you don't have to do any of this.
			t_sensor.pid = 0x01;
		}
		else if ((t_sensor.err <= 0) & (t_sensor.pid)) {
			CTIMER0->MR[0]  = (int) ((0)*INIT_COUNT_CTIMER);
			CTIMER0->TCR   &= ~(CTIMER_TCR_CEN_MASK);
			MRT0->CHANNEL[1].CTRL &= ~(MRT_CHANNEL_CTRL_INTEN_MASK);
			t_sensor.pid = 0;
			oled.stat &= ~(SSD1306_WRITE_MASK);
		}
	}
	else {
		// REMEMBER THAT CLEARING THE FLAG IS IMPORTANT.
		MRT0->CHANNEL[1].STAT = MRT_CHANNEL_STAT_INTFLAG_MASK;

		oled.stat |= (SSD1306_WRITE_MASK | SSD1306_STATE_MASK);

		// Proportional component.
		ep = (float) t_sensor.dat - setpoint;
		
		// Integral component.
		ei += ep;
		if (ei > 1) {
			ei = 1;
		}
		else if (ei < -1) {
			ei = -1;
		}
		
		// Derivative component - left alone for now.
		ed = ep - t_sensor.err;
		
		// Total error.
		pid_out = (Kp*ep) + (Ki*ei) + (Kd*ed);
		if (pid_out <= 1) {
			pid_out = 1;
		}
		else if (pid_out >= 120) {
			pid_out = 120;
		}


		// PID output TEST INPUT.
		ma.pwm = (float) (pid_out / ma.speed + placeh) / 2;

		// Motor turned ON.
		if (ma.pwm < 0.2) {
			ma.pwm = 0.2;
		}
		else if(ma.pwm > 1) {
			ma.pwm = 1;
		}

		placeh = ma.pwm;

		CTIMER0->MR[0]  = (int) ((ma.pwm)*INIT_COUNT_CTIMER);

		// Write previous error.
		t_sensor.err = ep;
	}	
}

void PIN_INT0_IRQHandler(void) {
	
	PINT->IST = 0x01;

	// Caculate the speed of the motor, accounting for gearing ratio.
	ma.position++;
	if (ma.tim >= 100) {
		ma.speed = (uint8_t) 30*(ma.position)/334;
		// Reset the position and time variables for next datum.
		ma.position = 0;
		ma.tim = 0; }
}

void PIN_INT1_IRQHandler(void) {

	PINT->IST = 0x02;

	// If the button is pressed, increment flag counter.
	counter++;
	oled.stat |= (SSD1306_WRITE_MASK | SSD1306_STATE_MASK);

	if (~(t_sensor.pid) & (counter > 0)) {
		// Set the fan speed to low.
		if (counter == LO) {
			CTIMER0->MR[0]  = (int) ((0.5)*INIT_COUNT_CTIMER);
			CTIMER0->TCR   |= (CTIMER_TCR_CEN_MASK);
		}
		// Set the fan speed to high.
		else if (counter == HI) {
			CTIMER0->MR[0]  = (int) ((1)*INIT_COUNT_CTIMER);
		}
		// Set the fan speed to auto.
		else {
			CTIMER0->MR[0]  = (int) ((0)*INIT_COUNT_CTIMER);
			//CTIMER0->TCR   &= ~(CTIMER_TCR_CEN_MASK);
			counter = 0;
		}
	}
}
// Main function.
int main(void) {

	// Disable interrupts.
	__disable_irq();
	NVIC_DisableIRQ(SysTick_IRQn);
	NVIC_DisableIRQ(I2C0_IRQn);
	NVIC_DisableIRQ(MRT0_IRQn);

	// Set the main system clock.
	SYSCON->MAINCLKSEL = (0x0<<SYSCON_MAINCLKSEL_SEL_SHIFT);
	// Update the main system clock.
	SYSCON->MAINCLKUEN &= ~(0x1);
	SYSCON->MAINCLKUEN |= (0x1);

	// Set the FRO frequency.
	BOARD_BootClockFRO24M();


	// Enable the Multi Rate Timer module.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_MRT_MASK);
	SYSCON->PRESETCTRL0    &= ~(SYSCON_PRESETCTRL0_MRT_RST_N_MASK);
	SYSCON->PRESETCTRL0    |= (SYSCON_PRESETCTRL0_MRT_RST_N_MASK);

	// Set the default MRT counter values.
	MRT0->CHANNEL[0].CTRL = (MRT_REPEAT << MRT_CHANNEL_CTRL_MODE_SHIFT | MRT_CHANNEL_CTRL_INTEN_MASK);
	MRT0->CHANNEL[0].INTVAL = IVALUE_CH0 &~ (MRT_CHANNEL_INTVAL_LOAD_MASK);


	// Turn on the Switch Matrix.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_SWM_MASK);

	// Turn on the CTimer module.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_CTIMER0_MASK);
	SYSCON->PRESETCTRL0    &= ~(SYSCON_PRESETCTRL0_CTIMER0_RST_N_MASK);
	SYSCON->PRESETCTRL0    |= (SYSCON_PRESETCTRL0_CTIMER0_RST_N_MASK);

	// Assign pins for the motor feedback signals.
	SWM0->PINASSIGN.PINASSIGN4 &= ~(0xFF);
	SWM0->PINASSIGN.PINASSIGN4 |=  (MOTORB);

	// Turn off the Switch Matrix.
	SYSCON->SYSAHBCLKCTRL0 &= ~(SYSCON_SYSAHBCLKCTRL0_SWM_MASK);

	// Set default values for the CTimer.
	CTIMER0->EMR   |= CTIMER_EMR_EM0_MASK;
	CTIMER0->PWMC  |= CTIMER_PWMC_PWMEN0_MASK;
	CTIMER0->MCR   &= ~(CTIMER_MCR_MR0R_MASK | CTIMER_MCR_MR0S_MASK | CTIMER_MCR_MR0I_MASK);
	CTIMER0->MCR   |= CTIMER_MCR_MR3R_MASK;
	CTIMER0->MR[3]  = INIT_COUNT_CTIMER;

	// Enable the GPIO module.
	GPIO_Config();

	// Set the SysTick frequency.
	SysTick_Config(120000);

	// Initialize the lm75 I2C.
	i2c_init();


	// Initialize the display.
	ssd1306_init();
	ssd1306_displaystr(SSD1306_INIT_GOOD);
	ssd1306_displaystr(LM75_INIT_GOOD);
	delay(1200000);
	i2c_starttx(SSD1306_ADDRESS);
	ssd1306_clear();
	i2c_stoptx();

	// Set motor flags and counters to zero.
	ma.pwm = 0;
	ma.position = 0;
	ma.speed = 0;

	// Turn off the display initially and set the display status flag to 0.
	i2c_starttx(SSD1306_ADDRESS);
	ssd1306_lowpowermode(0);
	oled.stat &= ~(SSD1306_STATE_MASK);

	// Initialize the WKT.
    	WKT_Config();
	// Enable the interrupts.
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_EnableIRQ(MRT0_IRQn);
	__enable_irq();

	// Main loop.
	while(1) {
		
		// Check if there is something to display on the OLED.
		if ((oled.stat) == (SSD1306_WRITE_MASK | SSD1306_STATE_MASK | SSD1306_INIT_MASK)) {

			// Turn the OLED on.
			if (oled.timeout == 0) {
				i2c_starttx(SSD1306_ADDRESS);
				ssd1306_lowpowermode(1);
			}

			// Else, display the sensor data.
			oled.currenty = 0;
			ssd1306_displaynum(t_sensor.dat);

			// Increment the display position index.
			oled.currenty = 2;

			// Display the current fan setting.
			if (counter == HI) {
				ssd1306_displaystr("Mode: HIGH ");
			}
			else if (counter == LO) {
				ssd1306_displaystr("Mode: LOW  ");
			}
			else {
				ssd1306_displaystr("Mode: AUTO ");
			}
			oled.stat &= ~(SSD1306_WRITE_MASK);
		}

		// Turn off the OLED.
		if (oled.timeout >= 8) {
			i2c_starttx(SSD1306_ADDRESS);
			ssd1306_lowpowermode(0);
			oled.stat &= ~(SSD1306_STATE_MASK);
			oled.timeout = 0;
		}
		else __asm("NOP");
		}
}
