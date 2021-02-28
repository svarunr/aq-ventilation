#include "ssd1306.h"
#include "stdio.h"
#include "ssd1306_couriern.h"

/* ================================================================================
   This struct defines the buffer as a 128*2 = 256, 1-byte/element array, which
   stores:
   1) the number of spaces in the given string.
   2) the formatted string in the buffer in terms of the bitmap ssd1306_couriern.h
   ================================================================================ */

typedef struct {
	uint8_t numspaces;
	uint8_t data[TWOLINE];
} buffer_s;

static buffer_s buffer;

/* ================================================================================
   This struct defines an OLED display instance, which stores:
   1) the current state of the display (ON/OFF),
   2) the initialization status (GOOD/BAD),
   3) the current x co-ordinate of the cursor,
   4) the curreny y co-ordinate of the cursor.
   ================================================================================ */

ssd1306 oled;


char temptostr[33];

/* ================================================================================
   This function turns OFF the SSD1306 display, assuming it has been called at the
   end of a write sequence.
   ================================================================================ */

void ssd1306_displayoff(void) {

	ssd1306_write(SSD1306_CM, SSD1306_CHRGPMP); ssd1306_write(SSD1306_CM, SSD1306_CHRPOFF); 
	ssd1306_write(SSD1306_CM, SSD1306_DISPOFF);
	i2c_stoptx();
	oled.stat &= ~(SSD1306_STATE_MASK);
}


/* ================================================================================
   This function turns ON the SSD1306 display, assuming it has been called at the
   beginning of a write sequence.
   ================================================================================ */

void ssd1306_displayon(void) {

	i2c_starttx(SSD1306_ADDRESS);
	ssd1306_write(SSD1306_CM, SSD1306_CHRGPMP); ssd1306_write(SSD1306_CM, SSD1306_CHRGPON);
	ssd1306_write(SSD1306_CM, SSD1306_DISPON);
	oled.stat |= (SSD1306_STATE_MASK);
}

/* ================================================================================
   This function puts the SSD1306 display in a low power mode by turning just the 
   display off, charge pump stays on. This function assumes it has been called 
   at the end of a write sequence.
   @param switch - low power mode ON (1) or OFF (0)?
   ================================================================================ */

void ssd1306_lowpowermode(bool flip) {

	ssd1306_write(SSD1306_CM, flip ? SSD1306_DISPON : SSD1306_DISPOFF);
	i2c_stoptx();
}

/* ================================================================================
   This function clears the _entire_ SSD1306 display. assuming it has been called
   before a transacation has begun.
   ================================================================================ */

void ssd1306_clear(void) {

	ssd1306_position(0,0);
	for (int k = 0; k < TWOLINE*2; k++) ssd1306_write(SSD1306_DM, 0x00);
}

/* ================================================================================
   This function clears the SSD1306 display incrementally from the start position
   to the end position by calculating the number of columns between them, and then
   clearing each column.
   @param startpage - page to start clearing from.
   @param stoppage  - page to clear to.
   @param startcol  - column to clear from.
   @param stopcol   - column to clear to.
   ================================================================================ */

void ssd1306_clearbw(uint8_t startpage, 
                     uint8_t stoppage, 
				     uint8_t startcol, 
					 uint8_t stopcol) {

	ssd1306_write(SSD1306_CM, SSD1306_PAGNUM);
	ssd1306_write(SSD1306_CM, startpage);  ssd1306_write(SSD1306_CM, stoppage); // Set the y co-ordinate.
	ssd1306_write(SSD1306_CM, SSD1306_COLNUM);
	ssd1306_write(SSD1306_CM, startcol);   ssd1306_write(SSD1306_CM, stopcol);  // Set the x co-ordinate.

	int lim = ((stopcol-startcol)+1)*((stoppage-startpage)+1);
	for(int k = 0; k < lim; k++) ssd1306_write(SSD1306_DM, 0x00);
}

/* ================================================================================
   This function sends data to the SSD1306 display. This must be used after calling
   ssd1306_starttx() and before calling ssd1306_stoptx() becuase it does not have
   functionality to start or end transmission.
   @param select - is the data a command or data to write to the GDDR?
   @param data   - the data to send.
   ================================================================================ */

void ssd1306_write(uint8_t select, 
                   uint8_t data) {

	WaitI2CPrimaryState(I2C0, I2C_STAT_MSTST_TXRDY);                    // Wait for I2C to be ready for transmission.
	I2C0->MSTDAT = select ? (SSD1306_COMMANDBYTE) : (SSD1306_DATABYTE); // Is it GDDR data or command?
	I2C0->MSTCTL = (MSTCTL_CONTINUE);                                   // Send the data.
	WaitI2CPrimaryState(I2C0, I2C_STAT_MSTST_TXRDY);
	I2C0->MSTDAT = (data);
	I2C0->MSTCTL = (MSTCTL_CONTINUE);
}

/* ================================================================================
   This function initializes the SSD1306 display with default starting point (0, 0)
   and following the spcifications provided in the data sheet.
   ================================================================================ */

void ssd1306_init(void) {

	i2c_starttx(SSD1306_ADDRESS);

	ssd1306_write(SSD1306_CM, SSD1306_DISPOFF); // Turn display OFF.
	ssd1306_write(SSD1306_CM, 0xD5); ssd1306_write(SSD1306_CM, 0x80); // Set display clock, to reset value.
	ssd1306_write(SSD1306_CM, 0xA8); ssd1306_write(SSD1306_CM, 0x1F); // Set MUX ratio, to 32.
	ssd1306_write(SSD1306_CM, 0xD3); ssd1306_write(SSD1306_CM, 0x00); // Set offset = 0.
	ssd1306_write(SSD1306_CM, 0x40); // Set display start register.
	ssd1306_write(SSD1306_CM, 0xA0); ssd1306_write(SSD1306_CM, 0xC0); // Set pins to the east of start point.
	ssd1306_write(SSD1306_CM, 0xDA); ssd1306_write(SSD1306_CM, 0x02); // Default COM pin re-map.
	ssd1306_write(SSD1306_CM, 0x81); ssd1306_write(SSD1306_CM, 0x7F); // Set contrast level.
	ssd1306_write(SSD1306_CM, 0xDB); ssd1306_write(SSD1306_CM, 0x40); // Set de-select level VCOM maximum.
	ssd1306_write(SSD1306_CM, 0xA4); // Output displays content on RAM.
	ssd1306_write(SSD1306_CM, 0xA6); // Set normal display, not inverted.
	ssd1306_write(SSD1306_CM, 0x20); ssd1306_write(SSD1306_CM, 0x00); // Set horizontal addressing mode.
	ssd1306_write(SSD1306_CM, SSD1306_PAGNUM); // Set the y co-ordinate.
	ssd1306_write(SSD1306_CM, 0x00);  ssd1306_write(SSD1306_CM, 0x03);
	ssd1306_write(SSD1306_CM, SSD1306_COLNUM); // Set the x co-ordinate.
	ssd1306_write(SSD1306_CM, 0x00); ssd1306_write(SSD1306_CM, 0x7F);
	ssd1306_write(SSD1306_CM, 0xD9); ssd1306_write(SSD1306_CM, 0xF1); // Set pre-charge period.
	ssd1306_write(SSD1306_CM, 0x8D); ssd1306_write(SSD1306_CM, 0x14); // Charge pump ON.
	ssd1306_write(SSD1306_CM, SSD1306_DISPON); // Turn display ON.

	ssd1306_clear();   // Clear entire screen.

	i2c_stoptx();

	oled.stat     |= (SSD1306_INIT_MASK); // Init successful.
	oled.stat     |= (SSD1306_STATE_MASK); // Display is ON.
	oled.currentx = 0; // Set default x.
	oled.currenty = 0; // Set default y.
}

/* ================================================================================
   This function sets the position on the display to write from.
   @param spage - the start page number, or y co-ordinate.
   @param scol  - the start column number, or x co-ordinate.
   ================================================================================ */

void ssd1306_position(uint8_t startpage, 
                      uint8_t startcol) {

	ssd1306_write(SSD1306_CM, SSD1306_PAGNUM);
	ssd1306_write(SSD1306_CM, startpage);  ssd1306_write(SSD1306_CM, 0x03); // Set the y co-ordinate.
	ssd1306_write(SSD1306_CM, SSD1306_COLNUM);
	ssd1306_write(SSD1306_CM, startcol);   ssd1306_write(SSD1306_CM, 0x7F);  // Set the x co-ordinate.
}

/* ================================================================================
   This function displays a string on the SSD1306 display by writing the contents
   of the buffer created in ssd1306_buffer to the display.
   @param spage - the start page number, or y co-ordinate.
   @param scol  - the start column number, or x co-ordinate.
   ================================================================================ */

void ssd1306_displaystr(char* message) {

	uint16_t count = 8*strlen(message) - 4*buffer.numspaces; // Each space is 4 less bytes.
	ssd1306_buffer(message);                                 // Write string to buffer.

	i2c_starttx(SSD1306_ADDRESS);

	ssd1306_position(oled.currenty, 0); // Hard set the x position to 0 for glitch prevention.

	for (uint16_t dstri = 0; dstri < count; dstri++) {
		if (buffer.data[dstri] == NEWLINE_FLAG) { ssd1306_position(++oled.currenty, 0); dstri++; }
		else { ssd1306_write(SSD1306_DM, buffer.data[dstri]); buffer.data[dstri] = 0; } 
	}

	i2c_stoptx();

	if (oled.currenty > 2) oled.currenty = 0;
}

/* ================================================================================
   This function creates a buffer, given a string, that can then be passed to
   ssd1306_write() to display on the SSD1306 display. The purpose of using a buffer
   is to occupy the I2C bus for as little time as possible - as opposed to doing the
   computations and then sending them immediately after, therefore prolonging the
   transaction.
   @param *str - pointer to the string that needs to be placed in buffer.
   ================================================================================ */

void ssd1306_buffer(char* str) {

	uint8_t j = 0, ind = 0, leng = strlen(str), overflow = leng - NUMCOLS/8;
	buffer.numspaces = 0;
	
	for (j = leng; j > 0; j--) {
		if (leng - j > overflow){
			if (str[j] == ' ') break; } }

	for (uint16_t bi = 0; bi < leng; bi++) {

		if (str[bi] == '\n' || str[bi] == '\0') { // Newline?
			buffer.data[ind] = NEWLINE_FLAG; ind++;
		}

		else if ((bi == j) & (j > 0)) { // Line wrapping.
			buffer.data[ind] = NEWLINE_FLAG; ind++;
		}

		else if (str[bi] == ' ') {
			for (uint8_t l = 0; l < 4; l++) { // Space?
				buffer.data[ind] = 0x00;
				ind++;
			}
			buffer.numspaces++;
		}

		else {
			uint16_t asc = 8*(str[bi] - 48);
			for (uint16_t k = asc; k < asc + 8; k++) {
				buffer.data[ind] = ssd1306_couriern[k]; ind++; } } } // Add to buffer.
}

/* ================================================================================
   This function displays a string on the SSD1306 display by writing the contents
   of the buffer created in ssd1306_buffer to the display.
   @param spage - the start page number, or y co-ordinate.
   @param scol  - the start column number, or x co-ordinate.
   ================================================================================ */

void ssd1306_displaynum(uint8_t num) {
	
	sprintf(&temptostr[0], "Temp: %d C", (uint8_t) num);

	ssd1306_displaystr(temptostr);
}

/* ================================================================================
   This function clears the buffer by writing 0's to it.
   ================================================================================ */

void ssd1306_clearbuffer(void) {

	for (uint16_t cbi = 0; cbi < TWOLINE; cbi++) buffer.data[cbi] = 0x00;
}

void delay_ms(uint16_t len) {

	uint32_t i = 0;
	uint32_t tm = len*1000;
	while (i<tm) i++;
}
