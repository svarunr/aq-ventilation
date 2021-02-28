/*
 * ccs811.c
 *
 *  Created on: Apr. 9, 2020
 *      Author: varunrathod
 */

#include "ccs811.h"

tvoc_sensor tv_sensor;

/** 
*   ================================================================================
*   \brief This function writes data to the CCS811.
*
*	There is a choice between setting the 802's MSTCTL register to either CONTINUE 
	or START. The CONTINUE state is used when telling the CCS811 that we want to 
	write data to it, the START state is used when telling the CCS811 that we want 
	to read data from it.
*   \param select 0 for START, 1 for CONTINUE.
*   \param data data that needs to be sent.
*/

void ccs811_write(uint8_t select, uint8_t data) {

	WaitI2CPrimaryState(I2C0, I2C_STAT_MSTST_TXRDY);
	I2C0->MSTDAT = (data);
	I2C0->MSTCTL = (select ? MSTCTL_CONTINUE : MSTCTL_START);
 }

/* ================================================================================
   This function tells the CCS811 to verify that the proper application is
   flashed to it.
   ================================================================================ */

void ccs811_appverif(void) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_APPVERIF);

	i2c_stoptx();
 }

/* ================================================================================
   This function tells the CCS811 to exit BOOT mode and enter APPLICATION mode.
   ================================================================================ */

void ccs811_appstart(void) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_APPSTART);

	i2c_stoptx();
 }

/* ================================================================================
   This function checks if the CCS811 reported any errors.
   ================================================================================ */

void ccs811_checkerr(void) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_CHECKERR);
	ccs811_write(0, (CCS811_ADDRESS << 1 | 1));

	while((I2C0->STAT & PRIMARY_STATE_MASK) != I2C_STAT_MSTST_RXRDY);

	tv_sensor.datbuf[0] = I2C0->MSTDAT;
	I2C0->MSTCTL = MSTCTL_CONTINUE;

	i2c_stoptx();
}

/* ================================================================================
   This function sets the period between measurements.
   @param interval - the interval between subsequent measurements.
   ================================================================================ */

void ccs811_measmode(uint8_t interval) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_MEASMODE);
	ccs811_write(1, interval);

	i2c_stoptx(); }

/* ================================================================================
   This function checks the status register of the CCS811.
   ================================================================================ */

void ccs811_checkstat(void) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_STATUSRG);
	ccs811_write(0, (CCS811_ADDRESS << 1 | 1));

	while((I2C0->STAT & PRIMARY_STATE_MASK) != I2C_STAT_MSTST_RXRDY);

	tv_sensor.datbuf[0] = I2C0->MSTDAT;
	I2C0->MSTCTL = MSTCTL_CONTINUE;

	i2c_stoptx();
}

/* ================================================================================
   This function reads data from the algorithm data register of the CCS811 and
   stores it in the global structure tv_sensor's datbuf field.
   ================================================================================ */

void ccs811_getdata(void) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_ALGMDATA);
	ccs811_write(0, (CCS811_ADDRESS << 1 | 1));

	for (uint8_t csi = 0; csi < 5; csi++) {
		while((I2C0->STAT & PRIMARY_STATE_MASK) != I2C_STAT_MSTST_RXRDY);

		tv_sensor.datbuf[csi] = I2C0->MSTDAT;
		I2C0->MSTCTL = MSTCTL_CONTINUE;
	}

	WaitI2CPrimaryState(I2C0, I2C_STAT_MSTST_RXRDY);
	while((I2C0->STAT & PRIMARY_STATE_MASK) != I2C_STAT_MSTST_RXRDY);

	tv_sensor.datbuf[5] = I2C0->MSTDAT;

	i2c_stoptx();

	tv_sensor.eco2 = (tv_sensor.datbuf[0] << 8) + (tv_sensor.datbuf[1]);
	tv_sensor.tvoc = (tv_sensor.datbuf[2] << 8) + (tv_sensor.datbuf[3]);
}

/* ================================================================================
   This function initializes the CCS811 in APPLICATION mode and with a measurement
   period of _period_. The possible values can be found in the header file.
   @param period - the interval between measurements.
   ================================================================================ */

void ccs811_init(uint8_t period) {

	ccs811_checkstat();

	if (!(tv_sensor.datbuf[0] & 0x9)) {
		ccs811_appverif();
		delay_ms(75);

		ccs811_appstart();

		ccs811_measmode(period);
	}
	return;
}

/* ================================================================================
   This function reads the hardware ID of the CCS811.
   ================================================================================ */

void ccs811_readhwid(void) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_HWID);
	ccs811_write(0, (CCS811_ADDRESS << 1 | 1));

	while((I2C0->STAT & PRIMARY_STATE_MASK) != I2C_STAT_MSTST_RXRDY);

	tv_sensor.datbuf[0] = I2C0->MSTDAT;
	I2C0->MSTCTL = MSTCTL_CONTINUE;

	i2c_stoptx();
 }

/* ================================================================================
   This function performs a software reset of the CCS811.
   ================================================================================ */

void ccs811_swreset(void) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_SWRESET);
	ccs811_write(1, 0x11);
	ccs811_write(1, 0xE5);
	ccs811_write(1, 0x72);
	ccs811_write(1, 0x8A);

	i2c_stoptx();
}

///* ================================================================================
//   This function writes the current temperature and humidity (obtained through
//   other sensors) to the CCS811. This is done by by first computing the values in
//   accordance with the data sheet. Assumes the data is given in float format.
//   @param temperature - current temperature from external sensors.
//   @param humidity - current humidity from external sensors.
//   ================================================================================ */
//void ccs811_envdata(uint16_t temperature, uint16_t humidity) {
//
//	i2c_starttx(CCS811_ADDRESS);
//
//	tv_sensor.datbuf[0] = humidity >> ;
//
//	i2c_stoptx();
//}

/* ================================================================================
   This function reads the raw data from the CCS811, the top 6 bits contain the
   value of the current through the sensor, while the bottom 10 bits are the voltage
   across the sensor.
   ================================================================================ */

void ccs811_rawdat(void) {

	i2c_starttx(CCS811_ADDRESS);

	ccs811_write(1, CCS811_RAWDATA);
	ccs811_write(0, (CCS811_ADDRESS << 1 | 1));

	while((I2C0->STAT & PRIMARY_STATE_MASK) != I2C_STAT_MSTST_RXRDY);

	tv_sensor.datbuf[0] = I2C0->MSTDAT;
	I2C0->MSTCTL = MSTCTL_CONTINUE;

	while((I2C0->STAT & PRIMARY_STATE_MASK) != I2C_STAT_MSTST_RXRDY);

	tv_sensor.datbuf[1] = I2C0->MSTDAT;

	i2c_stoptx();
}
