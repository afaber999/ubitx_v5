#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/**
 *  The second set of 16 pins on the Raduino's bottom connector are have the three clock outputs and the digital lines to control the rig.
 *  This assignment is as follows :
 *    Pin   1   2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 *         GND +5V CLK0  GND  GND  CLK1 GND  GND  CLK2  GND  D2   D3   D4   D5   D6   D7
 *  These too are flexible with what you may do with them, for the Raduino, we use them to :
 *  - TX_RX line : Switches between Transmit and Receive after sensing the PTT or the morse keyer
 *  - CW_KEY line : turns on the carrier for CW
 */

const byte PIN_CW_KEY = 2;
const byte PIN_TX_LPF_C = 3;
const byte PIN_TX_LPF_B = 4;
const byte PIN_TX_LPF_A = 5;
const byte PIN_CW_TONE = 6;
const byte PIN_TX_RX = 7;

const byte PIN_RS = 8;
const byte PIN_ENABLE = 9;
const byte PIN_D0 = 10;
const byte PIN_D1 = 11;
const byte PIN_D2 = 12;
const byte PIN_D3 = 13;

#endif
