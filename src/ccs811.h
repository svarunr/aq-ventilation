/*
 * ccs811.h
 *
 *  Created on: Apr. 9, 2020
 *      Author: varunrathod
 */

#ifndef CCS811_H_
#define CCS811_H_

#include <LPC802.h>
#include "lpc802_i2c.h"

/* The address of the CCS811. */
#define CCS811_ADDRESS (0x5B)

/* Register pointer addresses on the CCS811. */
#define CCS811_APPVERIF (0xF3)
#define CCS811_APPSTART (0xF4)
#define CCS811_CHECKERR (0xE0)
#define CCS811_MEASMODE (0x01)
#define CCS811_MEAS_1s  (0x10)
#define CCS811_STATUSRG (0x00)
#define CCS811_ALGMDATA (0x02)
#define CCS811_HWID     (0x20)
#define CCS811_SWRESET	(0xFF)
#define CCS811_RAWDATA  (0x03)

typedef struct tvoc_sensor {
	uint8_t  datbuf[6];
	uint16_t eco2;
	uint16_t tvoc;
} tvoc_sensor;

extern struct tvoc_sensor tv_sensor;

void ccs811_write(uint8_t, uint8_t);
void ccs811_appverif(void);
void ccs811_appstart(void);
void ccs811_getdata(void);
void ccs811_init(uint8_t);
void ccs811_measmode(uint8_t);
void ccs811_checkstat(void);
void ccs811_checkerr(void);
void ccs811_readhwid(void);

#endif /* CCS811_H_ */
