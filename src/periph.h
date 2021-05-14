/*
 * peripherals.h
 *
 *  Created on: Mar. 15, 2020
 *      Author: varunrathod
 */

#ifndef PERIPH_H_
#define PERIPH_H_

#include "LPC802.h"

#define BUTTON (8)
//#define LED1   ()
#define LED2   (9)
#define MOTORA (12)
#define MOTORB (4)
#define SENSEA (11)
#define SENSEB (13)

void GPIO_Config(void);

#endif /* PERIPH_H_ */
