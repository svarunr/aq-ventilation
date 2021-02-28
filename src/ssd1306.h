/*
 * ssd1306.h
 *
 *  Created on: Mar. 16, 2020
 *      Author: varunrathod
 */

#ifndef SSD1306_H_
#define SSD1306_H_

/* These are the libraries that are included for ssd1306.c functions. */
#include "lpc802_i2c.h"
#include "handle.h"
#include "lm75.h"
#include <stdlib.h>
#include <string.h>

/* This is the memory address of the SSD1306 display. */
#define SSD1306_ADDRESS 	(0x3C)

/* These constants are used when sending data to the SSD1306 display. */
#define SSD1306_DATABYTE 	(0xC0)
#define SSD1306_COMMANDBYTE (0x80)
#define SSD1306_CM          (0x01)
#define SSD1306_DM          (0x00)

/* These constants are used for display ON, OFF or low power mode. */
#define SSD1306_DISPOFF (0xAE)
#define SSD1306_DISPON  (0xAF)
#define SSD1306_CHRGPMP (0x8D)
#define SSD1306_CHRGPON (0x14)
#define SSD1306_CHRPOFF (0x10)

/* These constans are used to change the position of the current line. */
#define SSD1306_PAGNUM (0x22)
#define SSD1306_COLNUM (0x21)

/* These constants are used for for buffer calculations and formatting. */
#define NUMCOLS (128)
#define NUMROWS (2)
#define TWOLINE (NUMCOLS*NUMROWS)
#define NEWLINE_FLAG (0xFF)

/* These are masks for the status register. */
#define SSD1306_INIT_MASK  (0x01)
#define SSD1306_STATE_MASK (0x02)
#define SSD1306_WRITE_MASK (0x04) 

#define SSD1306_INIT_GOOD "OLED Display OK \n"

/* This struct is used to poll display parameters and flag setting. */
typedef struct ssd1306 {
	uint8_t stat;
	uint8_t timeout;
	uint8_t currentx;
	uint8_t currenty;
} ssd1306;

/* This statement makes the display instance _oled_ available to other source files. */
extern struct ssd1306 oled;

/* These are the function prototypes for ssd1306.c in the same order. */
void delay_ms(uint16_t);

void ssd1306_displayoff(void);
void ssd1306_displayon(void);
void ssd1306_lowpowermode(bool);
void ssd1306_clear(void);
void ssd1306_write(uint8_t, uint8_t);
void ssd1306_clearbw(uint8_t, uint8_t, uint8_t, uint8_t);
void ssd1306_init(void);
void ssd1306_position(uint8_t, uint8_t);
void ssd1306_displaystr(char*);
void ssd1306_displaynum(uint8_t);
void ssd1306_buffer(char*);
void ssd1306_clearbuffer(void);


#endif /* SSD1306_H_ */
