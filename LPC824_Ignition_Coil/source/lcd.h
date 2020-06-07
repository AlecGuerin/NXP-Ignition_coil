/**
 * @file lcd.h
 * @date 23 févr. 2020
 * @author Alec Guerin
 */

#ifndef LCD_H_
#define LCD_H_

#include "pin_mux.h"
#include "fsl_gpio.h"
#include "Font.h"

#define BUFFER_SIZE (1024)	///< Write Buffer size.

#define COLUMN_LSB				(uint8_t)0b00000000			///< Set Column Address LSB 0b0000xxxx.
#define COLUMN_MSB				(uint8_t)0b00010000			///< Set Column Address MSB 0b0001xxxx.
#define POWER_CTRL				(uint8_t)0b00101000			///< Set Power control 0b00101xxx.
#define SCROLL_LINE				(uint8_t)0b01000000			///< Set Scroll Line 0b01xxxxxx.
#define PAGE_ADDR				(uint8_t)0b10110000			///< Set Page Address 0b1011xxxx.
#define RESISTOR_RATIO			(uint8_t)0b00100000			///< Set Vlcd Resistor Ratio 0b00100xxx.
#define ELECTRONIC_VOLUME_1		(uint8_t)0b10000001			///< Set Electronic Volume (1/2) 0b10000001.
#define ELECTRONIC_VOLUME_2		(uint8_t)0b00000000 		///< Set Electronic Volume (2/2) 0b00xxxxxx.
#define ALL_PX_ON				(uint8_t)0b10100100			///< Set All-Pixel-ON 0x1010010x.
#define INVERS_DISPLAY			(uint8_t)0b10100110			///< Set Inverse Display 0b1010011x.
#define ENABLE_DISPLAY			(uint8_t)0b10101110			///< Set Display ENable 0b1010111x.
#define X_DIRECTION				(uint8_t)0b10100000			///< Set SEG Direction (X) 0b1010000x.
#define Y_DIRECTION				(uint8_t)0b11000000			///< Set COM Direction (Y) 0b1100x---.
#define ADV_PROG_CTL_1			(uint8_t)0b11111010			///< Set temperature compensation and Automatic column/row wrap round.
#define ADV_PROG_CTL_2			(uint8_t)0b10010100			///< Set temperature compensation and Automatic column/row wrap round.


#define ONE_BYTE	0x1			///< Number of byte to send for the SPI.

#define PIXEL_8X8_SIZE	8		///< Allocated memory in byte for a char into the font array.

#define SCREEN_HEIGHT	64		///< Screen height in pixel.
#define SCREEN_WIDTH	128		///< Screen width in pixel.
#define PAGE_HEIGHT		8		///< Page height in pixel. A page is the minimum manageable size for screen writing.

enum LCD_PIN		///< Enumeration used to store LCD pin in GPIO.
{
	IO_CS0 = 0,		///< GPIO pin for CS0.
	IO_CD = 4,		///< GPIO pin for CD.
	IO_RESET = 14,	///< GPIO pin for RESET.
	IO_SDA =17,		///< GPIO pin for SDA.
	IO_SCLK = 23	///< GPIO pin for SCLK.
};

/**
 * @brief Initialize the LCD screen.
 * Initialize the SPI as master, the LCD and display a picture for 3sec.
 * @return Initialization status.
 */
uint8_t LCD_Init();		///< Initialize the LCD.

/**
 * @brief Command to be sent through SPI.
 * @param cmd Buffer data to send.
 * @param size Size of the buffer.
 */
void LCD_WriteCommands(uint8_t cmd[], uint32_t size);	///< Write commands to the LCD.
/**
 * @brief Write one byte command.
 * @param cmd Command to send.
 */
void LCD_WriteCommand(uint8_t cmd);		///< Write one byte command to the LCD.

/**
 * @brief Write the number of byte provided of the provide data buffer.
 * @param data Data buffer.
 * @param size Number of byte to send from index '0'.
 */
void LCD_WriteData(uint8_t data[], uint32_t size);	///< Write data to the LCD.
/**
 * @brief Write one data byte.
 * @param cmd Byte to send.
 */
void LCD_WriteOneData(uint8_t cmd);		///< Write one byte data to the LCD.
/**
 * @brief Write the number of byte provided from the staring index of the provide data buffer.
 * @param data Data buffer.
 * @param size Number of bytes to send.
 * @param startIndex Data buffer starting index.
 */
void LCD_WriteDataFromStart(uint8_t data[], uint32_t size, uint32_t startIndex);	///< Write commands at the provided starting index to the LCD.
/**
 * @brief Select a column in the screen to send data
 * @param col Column index.
 */
void LCD_SelectColumn(uint8_t col);		///< Select the LCD cursor column.
/**
 * @brief Send a couple of byte to fill the screen.
 * @param data1 First data to set.
 * @param data2 Second data to set
 */
void LCD_DisplayClear(uint8_t data1, uint8_t data2);	///< Display on screen by packet of 2 the provided data.
/**
 * @brief Display a string at the provided position.
 * @param y 'Y' position [page].
 * @param x 'X' position [px].
 * @param string Message to write.
 */
void LCD_DisplayString(uint8_t yPos, uint8_t xPos, char *string);	///< Display a string at the provided position.

/**
 * @brief Display a raw picture on selected position.
 * @param x0 'X' position [px].
 * @param y0 'Y' position [page].
 * @param pic Picture to display.
 */
void LCD_DisplayPicture(uint8_t x0, uint8_t y0, T_picture pic);		///< Display a picture at the provided position.
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
void LCD_DisplayPartPicture(uint8_t x0, uint8_t y0, uint8_t xStart, uint8_t yStart, uint8_t width, uint8_t height, T_picture pic); ///< Display a part of a picture at the provided position.
/**
 * @brief Display a compressed picture on selected position.
 * @param x0 'X' position [px].
 * @param y0 'Y' position [page].
 * @param pic Picture to display.
 */
void LCD_DisplayCompressPicture(uint8_t x0, uint8_t y0, T_picture pic);		///< Display a compressed picture at the provided position.
/**
 * @brief Display a inverse compressed picture on selected position.
 * @param x0 'X' position [px].
 * @param y0 'Y' position [page].
 * @param pic Picture to display.
 */
void LCD_DisplayCompressPictureI(uint8_t x0, uint8_t y0, T_picture pic);	///< Display a color inverse picture at the provided position.
/**
 * @brief Display a rectangle with provided color on selected area.
 * @param x0 Up Left corner of the rectangle [px].
 * @param y0 Up Left corner of the rectangle [line].
 * @param width	Rectangle width [px]
 * @param height Rectangle height [line].
 * @param color Color to set in rectangle.
 */
void LCD_DisplayRectangle(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height, uint8_t color);	///< Display a rectangle with the provided color.
/**
 * @brief Set a pixel at the provided place.
 * @remark A root page is necessary because the minimum writable size is 8 verticals pixels [px].
 * @param x X position [px].
 * @param y Y position [px].
 * @param rootPage Byte corresponding to the 8 vertical pixels where the pixel have to be set.
 * @return The update provided page.
 */
uint8_t LCD_DisplaySetPixel(uint8_t x, uint8_t y, uint8_t rootPage);		///< Set a pixel at provided position.
/**
 * @brief Display a progress bar with a 100px length.
 * @param x0 X position of the progress bar [px].
 * @param y0 Y position of the progress bar [line].
 * @param value Progress bar value to set [0-100].
 */
void LCD_ProgressBar(uint8_t x0, uint8_t y0, uint8_t value);		///< Display a progress bar with value from 0 to 100.

#endif /* LCD_H_ */
