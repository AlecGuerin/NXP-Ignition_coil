/**
 * @file lcd.c
 * @date 23 févr. 2020
 * @author Alec Guerin
 */
#include "lcd.h"
#include "fsl_spi.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "Font.h"

extern T_picture IMG_sevenLogo_c;	///< 'Lotus Seven' picture from "Font.h"

// SPI transfer structure to use for sending data.
spi_transfer_t _transfert;

/**
 * @brief Initialize the LCD screen.
 * Initialize the SPI as master, the LCD and display a picture for 3sec.
 * @return Initialization status.
 */
uint8_t LCD_Init(){

	spi_master_config_t userConfig;
	uint32_t srcFreq = 0U;
	uint32_t dataSize = 8;

	// Set the initialization commands to send.
	uint8_t cmd[] = {
			X_DIRECTION,
			(uint8_t)(Y_DIRECTION | (0x1F & 0x08)),
			(uint8_t)(RESISTOR_RATIO | 0b100),
			(uint8_t)(ELECTRONIC_VOLUME_1),
			(uint8_t)(ELECTRONIC_VOLUME_2 | 0x0C),
			(uint8_t)ADV_PROG_CTL_1,
			(uint8_t)ADV_PROG_CTL_2,
			(uint8_t)(POWER_CTRL | 0b100)
	};

	SPI_MasterGetDefaultConfig(&userConfig);
	userConfig.baudRate_Bps = 1000000;
	userConfig.sselNumber   = 0;
	srcFreq					= CLOCK_GetFreq(kCLOCK_Irc);

	if(SPI_MasterInit(SPI0, &userConfig, srcFreq) != kStatus_Success){
		return kStatus_Fail;
	}
	// Initialize the configuration flag.
	_transfert.configFlags = kSPI_ReceiveIgnore | kSPI_EndOfTransfer;

	// Reset LCD
	GPIO_PinWrite(GPIO, 0, IO_RESET, 1);
	CLOCK_Delay(10000);
	GPIO_PinWrite(GPIO, 0, IO_RESET, 0);
	CLOCK_Delay(10000);
	GPIO_PinWrite(GPIO, 0, IO_RESET, 1);
	CLOCK_Delay(10000);

	// Write LCD parameters to use.
	LCD_WriteCommands(cmd, dataSize);

	// Voltage Follower Circuit ON.
	CLOCK_Delay(10000);
	LCD_WriteOneCommand(POWER_CTRL | 0b110);

	// Voltage Follower Circuit ON.
	CLOCK_Delay(10000);
	LCD_WriteOneCommand(POWER_CTRL | 0b111);

	// Display ON Set Display enable.
	CLOCK_Delay(10000);
	LCD_WriteOneCommand(ENABLE_DISPLAY | 1);
	// Display picture.
	LCD_DisplayCompressPictureI(0,0,IMG_sevenLogo_c);
	// Wait to show picture.
	CLOCK_Delay(300000);
	return kStatus_Success;
}

/**
 * @brief Command to be sent through SPI.
 * @param cmd Buffer data to send.
 * @param size Size of the buffer.
 */
void LCD_WriteCommands(uint8_t cmd[], uint32_t size)
{
	// Activate CD = command
	GPIO_PinWrite(GPIO, 0, IO_CD, 0);

	_transfert.txData = cmd;
	_transfert.dataSize = size;
	SPI_MasterTransferBlocking(SPI0, &_transfert);
}

/**
 * @brief Write one byte command.
 * @param cmd Command to send.
 */
void LCD_WriteOneCommand(uint8_t cmd){

	uint8_t cmdData[] = {cmd};
	LCD_WriteData(cmdData, 1);
}

/**
 * @brief Write the number of byte provided of the provide data buffer.
 * @param data Data buffer.
 * @param size Number of byte to send from index '0'.
 */
void LCD_WriteData(uint8_t data[], uint32_t size)
{
	GPIO_PinWrite(GPIO, 0, IO_CD, 1);

	_transfert.txData = data;
	_transfert.dataSize = size;
	SPI_MasterTransferBlocking(SPI0, &_transfert);
}

/**
 * @brief Write the number of byte provided from the staring index of the provide data buffer.
 * @param data Data buffer.
 * @param size Number of bytes to send.
 * @param startIndex Data buffer starting index.
 */
void LCD_WriteDataFromStart(uint8_t data[], uint32_t size, uint32_t startIndex)
{
	GPIO_PinWrite(GPIO, 0, IO_CD, 1);

	_transfert.txData = data + startIndex;
	_transfert.dataSize = size;
	SPI_MasterTransferBlocking(SPI0, &_transfert);
}

/**
 * @brief Write one data byte.
 * @param cmd Byte to send.
 */
void LCD_WriteOneData(uint8_t cmd){

	uint8_t cmdData[] = {cmd};
	LCD_WriteData(cmdData, 1);
}

/**
 * @brief Select a column in the screen to send data
 * @param col Column index.
 */
void LCD_SelectColumn(uint8_t col)
{
	// Get 4 first bits of  "col" OR COLUMN_LSB
	LCD_WriteOneCommand(COLUMN_LSB | (0x000F & col));
	// Get next 4 bits of "col" OR COLUMN_MSB
	LCD_WriteOneCommand(COLUMN_MSB | (0x000F & (col >> 4)));
}

/**
 * @brief Send a couple of byte to fill the screen.
 * @param data1 First data to set.
 * @param data2 Second data to set
 */
void LCD_DisplayClear(uint8_t data1, uint8_t data2)
{
	uint8_t cmd[SCREEN_WIDTH];
	uint8_t i = 0;

	for (i=0; i < SCREEN_WIDTH; i += 2){
		cmd[i] = data1;
		cmd[i+1] = data2;
	}

	// Write 8 pages of 128 column of 8 pixels
	for (i=0; i < PAGE_HEIGHT; i++) {
		// Write 1 Page of 128 column of 8 pixels
		LCD_WriteOneCommand(PAGE_ADDR | i);	// Set Page address (up to 8 pages)
		LCD_SelectColumn(0);

		LCD_WriteData(cmd,SCREEN_WIDTH);
	}
}

/**
 * @brief Display a string at the provided position.
 * @param y 'Y' position [page].
 * @param x 'X' position [px].
 * @param string Message to write.
 */
void LCD_DisplayString(uint8_t y, uint8_t x, char *string)
{
	int i, j;
	uint8_t str_length;
	unsigned char *one_char;
	unsigned char *charSize;

	str_length = strlen(string);

	// Select current working page
	LCD_WriteOneCommand(PAGE_ADDR | y);
	// Select first column of line
	LCD_SelectColumn(x);

	for (i = 0; i < str_length; i++)
	{
		// 1 character is 8x8 pixels (8 byte because very bits correspond to a pixel)
		one_char = (unsigned char *) &Font_TAB[(string[i] - (char) 24) * PIXEL_8X8_SIZE]; // We remove 24 because the font array start at the 24th index of the ASCII table
		// First byte is used as size of displayed char
		charSize = one_char;
		// Increment pointer
		*one_char++;

		// Write max 7 columns
		for (j = 0; j < *charSize; j++)
			LCD_WriteOneData(*one_char++);

		// Add a space at the end
		LCD_WriteOneData(0x00);
	}
}

/**
 * @brief Display a raw picture on selected position.
 * @param x0 'X' position [px].
 * @param y0 'Y' position [page].
 * @param pic Picture to display.
 */
void LCD_DisplayPicture(uint8_t x0, uint8_t y0, T_picture pic) {

	int i = y0;

	// Get starting line and increment until last one
	for (; i < y0 + pic.HEIGHT; i++) {
		// Select current working page
		LCD_WriteCommand(PAGE_ADDR | i);
		// Select first column of line
		LCD_SelectColumn(x0);

		LCD_WriteDataFromStart(pic.DATA, pic.WIDTH, x0);
	}
}

/**
 * @brief Display a part of raw picture on selected position.
 * @param x0 'X' position of the full picture [px].
 * @param y0 'Y' position of the full picture [page].
 * @param xStart 'X' Starting position in the picture.
 * @param yStart 'Y' starting position in the picture.
 * @param width Width of the part to display [px].
 * @param height Height of the picture to display [page].
 * @param pic Picture to display.
 */
void LCD_DisplayPartPicture(uint8_t x0, uint8_t y0, uint8_t xStart, uint8_t yStart, uint8_t width, uint8_t height, T_picture pic)
{
	int i, j;
	int index = yStart * pic.WIDTH + xStart;

	// Get starting line and increment until last one
	for (i = y0 + yStart; i < y0 + yStart + height; i++)
	{
		// Select current working page
		LCD_WriteOneCommand(PAGE_ADDR |i);
		// Select first column of line
		LCD_SelectColumn(x0+xStart);

		// Compute next starting index index
		index =  i * pic.WIDTH + xStart;

		// Write data on each column
		for (j = x0 + xStart; j < x0 + xStart + width; j++)
		{
			LCD_WriteOneData(pic.DATA[index++]);
		}
	}
}

/**
 * @brief Display a compressed picture on selected position.
 * @param x0 'X' position [px].
 * @param y0 'Y' position [page].
 * @param pic Picture to display.
 */
void LCD_DisplayCompressPicture(uint8_t x0, uint8_t y0, T_picture pic) {
	int i = 0, j = 1, k;
	int index = 0;
	// The loop use double indexes to limit the operation 'i+1' that should be performed on second loop
	for (;  j < pic.DATA_LENGTH; i += 2, j +=2) {
		// Add the number of same byte
		for (k = 0; k < pic.DATA[i]; k++) {
			// Change line when picture width is reached
			if (index % pic.WIDTH == 0) {
				LCD_SelectColumn(x0);
				LCD_WriteCommand(PAGE_ADDR | y0);
				y0++;
			}
			// Write data
			LCD_WriteOneData(pic.DATA[j]);
			index++;
		}
	}
}

/**
 * @brief Display a inverse color compressed picture on selected position.
 * @param x0 'X' position [px].
 * @param y0 'Y' position [page].
 * @param pic Picture to display.
 */
void LCD_DisplayCompressPictureI(uint8_t x0, uint8_t y0, T_picture pic) {
	int i = 0, j = 1, k;
	int index = 0;
	// The loop use double indexes to limit the operation 'i+1' that should be performed on second loop
	for (;  j < pic.DATA_LENGTH; i += 2, j +=2) {
		// Add the number of same byte
		for (k = 0; k < pic.DATA[i]; k++) {
			// Change line when picture width is reached
			if (index % pic.WIDTH == 0) {
				LCD_SelectColumn(x0);
				LCD_WriteOneCommand(PAGE_ADDR | y0);
				y0++;
			}
			// Write data
			LCD_WriteOneData(~pic.DATA[j]); // Inverse color
			index++;
		}
	}
}

/**
 * @brief Display a rectangle with provided color on selected area.
 * @param x0 Up Left corner of the rectangle [px].
 * @param y0 Up Left corner of the rectangle [line].
 * @param width	Rectangle width [px]
 * @param height Rectangle height [line].
 * @param color Color to set in rectangle.
 */
void LCD_DisplayRectangle(uint8_t x0, uint8_t y0, uint8_t width,
		uint8_t height, uint8_t color) {
	int i, j;
	/*uint8_t data[width];

	for (i = 0; i <  width; i++) {
		data[i] = color;
	}*/

	// Get starting line and increment until last one
	for (i = y0; i < y0 + height; i++) {
		// Select current working page
		LCD_WriteOneCommand(PAGE_ADDR | i);
		// Select first column of line
		LCD_SelectColumn(x0);
		//LCD_WriteDataFromStart(data, width, x0);
		// Write data on each columns
		for (j = x0; j < x0 + width; j++) {
			LCD_WriteOneData(color);
		}
	}
}

/**
 * @brief Set a pixel at the provided place.
 * @remark A root page is necessary because the minimum writable size is 8 verticals pixels [px].
 * @param x X position [px].
 * @param y Y position [px].
 * @param rootPage Byte corresponding to the 8 vertical pixels where the pixel have to be set.
 * @return The update provided page.
 */
uint8_t LCD_DisplaySetPixel(uint8_t x, uint8_t y, uint8_t rootPage) {

	// Set on the page the corresponding pixel.
	rootPage |= (1 << y % 8);
	LCD_WriteCommand(PAGE_ADDR | (y / 8));
	LCD_SelectColumn(x);

	LCD_WriteOneData(rootPage);
	return rootPage;
}

/**
 * @brief Display a progress bar with a 100px length.
 * @param x0 X position of the progress bar [px].
 * @param y0 Y position of the progress bar [line].
 * @param value Progress bar value to set [0-100].
 */
void LCD_ProgressBar(uint8_t x0, uint8_t y0, uint8_t value) {
	// progress bar is compose of 2 type of values: 'full' as '|' and empty as ':'.
	// the result will look like : ||||||::::::|
	uint8_t passed = 0x7E;	//[|]
	uint8_t nok = 0x42;		//[:]
	uint8_t size = 100;
	uint8_t i = 1;

	uint8_t data[size + 2];

	// Set end of progress bar
	data[0] = passed;
	data[size + 1] = passed;

	// Make sure that the provided value doesn't exceed the size.
	if (value > size)
		value = size;

	value ++;
	for(i =1; i < value; i++){
		data[i] = passed;
	}
	size ++;
	for(i = value; i < size; i++){
		data[i] = nok;
	}
	size ++;

	// Select provided page addr
	LCD_WriteCommand(PAGE_ADDR | y0);
	// Select provided column
	LCD_SelectColumn(x0);

	LCD_WriteDataFrom(data, size, x0);
}
