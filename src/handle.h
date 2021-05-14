/*
 * handle.h
 *
 *  Created on: Mar. 15, 2020
 *      Author: varunrathod
 */

#ifndef HANDLE_H_
#define HANDLE_H_

#include "LPC802.h"
#include "clock_config.h"
#include "periph.h"
#include "lm75.h"

/* This struct is used to poll display parameters and flag setting. */
typedef struct motor {
	float    pwm;
	uint32_t position;
	uint8_t  speed;
	uint8_t  tim;
} motor;

/* This statement makes the display instance _oled_ available to other source files. */
extern struct motor ma;

/* This is an enum type for the motor modes. */
typedef enum {
	AUTO,
	LOW,
	HIGH
} motor_mode;

// WKT values.
#define WKT_RELOAD (1000000)

void SysTickHandler();
void WKT_IRQHandler();
void WKT_Config();

#endif /* HANDLE_H_ */
