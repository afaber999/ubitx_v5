/**
 * The user interface of the ubitx consists of the encoder, the push-button on top of it
 * and the 16x2 LCD display.
 * The upper line of the display is constantly used to display frequency and status
 * of the radio. Occasionally, it is used to provide a two-line information that is
 * quickly cleared up.
 */

#include "global.h"
#include <LiquidCrystal.h>

/**
 * The Raduino board is the size of a standard 16x2 LCD panel. It has three connectors:
 *
 * First, is an 8 pin connector that provides +5v, GND and six analog input pins that can also be
 * configured to be used as digital input or output pins. These are referred to as A0,A1,A2,
 * A3,A6 and A7 pins. The A4 and A5 pins are missing from this connector as they are used to
 * talk to the Si5351 over I2C protocol.
 *
 * Second is a 16 pin LCD connector. This connector is meant specifically for the standard 16x2
 * LCD display in 4 bit mode. The 4 bit mode requires 4 data lines and two control lines to work:
 * Lines used are : RESET, ENABLE, D4, D5, D6, D7
 * We include the library and declare the configuration of the LCD panel too
 */

static LiquidCrystal lcd(PIN_RS, PIN_ENABLE, PIN_D0, PIN_D1, PIN_D2, PIN_D3);

void initDisplay()
{
  lcd.begin(16, 2); // initialize the lcd for 16 chars 2 lines
  lcd.clear();
}

/**
 * Meter (not used in this build for anything)
 * the meter is drawn using special characters. Each character is composed of 5 x 8 matrix.
 * The  s_meter array holds the definition of the these characters.
 * each line of the array is is one character such that 5 bits of every uint8_t
 * makes up one line of pixels of the that character (only 5 bits are used)
 * The current reading of the meter is assembled in the string called meter
 */

char meter[17];

const uint8_t PROGMEM s_meter_bitmap[] = {
    B00000, B00000, B00000, B00000, B00000, B00100, B00100, B11011,
    B10000, B10000, B10000, B10000, B10100, B10100, B10100, B11011,
    B01000, B01000, B01000, B01000, B01100, B01100, B01100, B11011,
    B00100, B00100, B00100, B00100, B00100, B00100, B00100, B11011,
    B00010, B00010, B00010, B00010, B00110, B00110, B00110, B11011,
    B00001, B00001, B00001, B00001, B00101, B00101, B00101, B11011,
    B10000, B11000, B11100, B11110, B11100, B11000, B10000, B00000,
    B00001, B00011, B00111, B01111, B00111, B00011, B00001, B00000};

// initializes the custom characters
// we start from char 1 as char 0 terminates the string!
void initMeter()
{
  lcd.createChar(1, (uint8_t *)(s_meter_bitmap));
  lcd.createChar(2, (uint8_t *)(s_meter_bitmap + 8));
  lcd.createChar(3, (uint8_t *)(s_meter_bitmap + 16));
  lcd.createChar(4, (uint8_t *)(s_meter_bitmap + 24));
  lcd.createChar(5, (uint8_t *)(s_meter_bitmap + 32));
  lcd.createChar(6, (uint8_t *)(s_meter_bitmap + 40));
  lcd.createChar(0, (uint8_t *)(s_meter_bitmap + 48));
  lcd.createChar(7, (uint8_t *)(s_meter_bitmap + 56));
}

/**
 * The meter is drawn with special characters.
 * character 1 is used to simple draw the blocks of the scale of the meter
 * characters 2 to 6 are used to draw the needle in positions 1 to within the block
 * This displays a meter from 0 to 100, -1 displays nothing
 */

void drawMeter(int8_t needle)
{
  int16_t i, s;

  if (needle < 0)
    return;

  s = (needle * 4) / 10;
  for (i = 0; i < 8; i++)
  {
    if (s >= 5)
      meter[i] = 1;
    else if (s >= 0)
      meter[i] = 2 + s;
    else
      meter[i] = 1;
    s = s - 5;
  }
  if (needle >= 40)
    meter[i - 1] = 6;
  meter[i] = 0;
}

// The generic routine to display one line on the LCD
void printLine(int linenmbr, const char *c)
{
  if (strcmp(c, printBuff[linenmbr]))
  {                             // only refresh the display when there was a change
    lcd.setCursor(0, linenmbr); // place the cursor at the beginning of the selected line
    lcd.print(c);
    strcpy(printBuff[linenmbr], c);

    for (uint8_t i = strlen(c); i < 16; i++)
    { // add white spaces until the end of the 16 characters line is reached
      lcd.print(' ');
    }
  }
}

//  short cut to print to the first line
void printLine1(const char *c)
{
  printLine(1, c);
}
//  short cut to print to the first line
void printLine2(const char *c)
{
  printLine(0, c);
}

// this builds up the top line of the display with frequency and mode
void updateDisplay()
{
  // tks Jack Purdum W8TEE
  // replaced fsprint commmands by str commands for code size reduction

  memset(cBuf, 0, sizeof(cBuf));
  memset(bBuf, 0, sizeof(bBuf));

  ultoa(settings.frequency, bBuf, DEC);

  if (settings.inTx)
  {
    cBuf[0] = ' ';
    cBuf[1] = ' ';
    cBuf[2] = ' ';
    if (settings.cwTimeout > 0)
    {
      cBuf[3] = 'C';
      cBuf[4] = 'W';
      cBuf[5] = ':';
    }
    else
    {
      cBuf[3] = 'T';
      cBuf[4] = 'X';
      cBuf[5] = ':';
    }
  }
  else
  {
    if (settings.ritOn)
    {
      cBuf[0] = 'R';
      cBuf[1] = 'I';
      cBuf[2] = 'T';
    }
    else
    {
      cBuf[0] = (settings.isUSB) ? 'U' : 'L';
      cBuf[1] = 'S';
      cBuf[2] = 'B';
    }
    cBuf[3] = ' ';
    cBuf[4] = (settings.vfoActive == VFO_A) ? 'A' : 'B';
    cBuf[5] = ':';
  }

  // one mhz digit if less than 10 M, two digits if more
  if (settings.frequency < 10000000l)
  {
    cBuf[ 6] = ' ';
    cBuf[ 7] = bBuf[0];
    cBuf[ 8] = '.';
    cBuf[ 9] = bBuf[1];
    cBuf[10] = bBuf[2];
    cBuf[11] = bBuf[3];
    cBuf[12] = '.';
    cBuf[13] = bBuf[4];
    cBuf[14] = bBuf[5];
    cBuf[15] = bBuf[6];
  }
  else
  {
    cBuf[ 6] = bBuf[0];
    cBuf[ 7] = bBuf[1];
    cBuf[ 8] = '.';
    cBuf[ 9] = bBuf[2];
    cBuf[10] = bBuf[3];
    cBuf[11] = bBuf[4];
    cBuf[12] = '.';
    cBuf[13] = bBuf[5];
    cBuf[14] = bBuf[6];
    cBuf[15] = bBuf[67];
  }

  // AF TODO CHECK OUTSIDE LCD PRINT LIMITS
  // if (settings.inTx)
  //   strcat(cBuf, " TX");
  printLine(1, cBuf);

}
