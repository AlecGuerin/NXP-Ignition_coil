/**
 * @file Font.h
 *
 * @brief Contains fonts and pictures to display.
 * @date 2019
 * @author Elyctis
 *
 * Contains two different fonts:\n
 * 		- Standard : Full ASCII table + 8 specials characters.\n
 * 		- Upper case bold : Only upper case char [A-Z].\n
 * Contains pictures:\n
 * 		- 'Lotus Seven' logo [Compressed].\n
 * 		- Clock [Raw]\n
 * 		- Check [Raw]\n
 * 		- Cross [Raw]
 *
 */

#ifndef INC_FONT_H_
#define INC_FONT_H_

/**
 * @brief Structure used to store picture data.
 */
typedef struct
{
	uint8_t HEIGHT;			///< Height of the image [page].
	uint8_t WIDTH;			///< Width of the image [px].
	uint8_t *DATA;			///< Array where data are stored.

	int DATA_LENGTH;		///< In case of compressed data, used to provide the compressed array size.

}T_picture;

extern T_picture IMG_sevenLogo_c;		///< Type that contains the Seven logo data.

extern T_picture IMG_clock;		///< Type that contains a clock picture.
extern T_picture IMG_check;		///< Type that contains a check print picture.
extern T_picture IMG_cross;		///< Type that contains a cross print picture.

/**
 * @brief Array that contains the MAJ BOLD font data.
 *
 * Array that contains only the upper case bold font data. It start with the ASCII value 0x40'@' and finish with the value 0x5B '['.
 */
extern const unsigned char Bold_Maj_Font_TAB[];

/**
 * @brief Array that contains the 'default' font data + special chars.
 *
 * The array contains the ASCII table from char  0x20 'space' to 0x7F 'DEL'.\n
 * The first 8 chars 0x17 to 0x1F are special chars from extended ASCII table (CP437):\n
 *  	- 0x18 '\30' => 0x20 'space'\n
 *  	- 0x19 '\31' => 0x81 'ü'\n
 *  	- 0x1A '\32' => 0x82 'é'\n
 *  	- 0x1B '\33' => 0x85 'à'\n
 *  	- 0x1C '\34' => 0x24 '$'\n
 *  	- 0x1D '\35' => 0x9C '£'\n
 *  	- 0x1E '\36' => 0x9D '¥'\n
 *  	- 0x1F '\37' => 0x-- '€'\n
 */
extern const unsigned char Font_TAB[];

 #endif/* INC_FONT_H_ */
