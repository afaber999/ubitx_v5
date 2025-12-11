/**
 * This source file is under General Public License version 3.
 *
 */

#include "global.h"
#include <Wire.h>
#include <EEPROM.h>

settings_t settings;

// function prototypes for functions used only in this file
static void checkButton();

// temp buffer to build strings for the display
char cBuf[30];
char bBuf[30];
char printBuff[2][31]; // mirrors what is showing on the two lines of the display
int count = 0;         // to generally count ticks, loops, etc

uint32_t usbCarrier;
char isUsbVfoA = 0;
char isUsbVfoB = 1;
uint32_t ritRxFrequency, ritTxFrequency; // frequency is the current frequency on the dial
uint32_t firstIF = 45005000L;

// these are variables that control the keyer behaviour
bool Iambic_Key = true;
uint8_t keyerControl = IAMBICB;

/**
 * Raduino needs to keep track of current state of the transceiver. These are a few variables that do it
 */
// frequency when it crosses the frequency border of 10 MHz
uint32_t dbgCount = 0;      // not used now
unsigned char txFilter = 0; // which of the four transmit filters are in use
                            // beat frequency

/**
 * Below are the basic functions that control the uBitx. Understanding the functions before
 * you start hacking around
 */

/**
 * Our own delay. During any delay, the raduino should still be processing a few times.
 */

void active_delay(uint32_t delay_by)
{
  uint32_t timeStart = millis();

  while (millis() - timeStart <= delay_by)
  {
    // Background Work
    checkCAT();
  }
}

/**
 * Select the properly tx harmonic filters
 * The four harmonic filters use only three relays
 * the four LPFs cover 30-21 Mhz, 18 - 14 Mhz, 7-10 MHz and 3.5 to 5 Mhz
 * Briefly, it works like this,
 * - When KT1 is OFF, the 'off' position routes the PA output through the 30 MHz LPF
 * - When KT1 is ON, it routes the PA output to KT2. Which is why you will see that
 *   the KT1 is on for the three other cases.
 * - When the KT1 is ON and KT2 is off, the off position of KT2 routes the PA output
 *   to 18 MHz LPF (That also works for 14 Mhz)
 * - When KT1 is On, KT2 is On, it routes the PA output to KT3
 * - KT3, when switched on selects the 7-10 Mhz filter
 * - KT3 when switched off selects the 3.5-5 Mhz filter
 * See the circuit to understand this
 */

void setTXFilters(uint32_t frequency)
{

  if (frequency > 21000000L)
  { // the default filter is with 35 MHz cut-off
    digitalWrite(PIN_TX_LPF_A, 0);
    digitalWrite(PIN_TX_LPF_B, 0);
    digitalWrite(PIN_TX_LPF_C, 0);
  }
  else if (frequency >= 14000000L)
  { // thrown the KT1 relay on, the 30 MHz LPF is bypassed and the 14-18 MHz LPF is allowd to go through
    digitalWrite(PIN_TX_LPF_A, 1);
    digitalWrite(PIN_TX_LPF_B, 0);
    digitalWrite(PIN_TX_LPF_C, 0);
  }
  else if (frequency > 7000000L)
  {
    digitalWrite(PIN_TX_LPF_A, 0);
    digitalWrite(PIN_TX_LPF_B, 1);
    digitalWrite(PIN_TX_LPF_C, 0);
  }
  else
  {
    digitalWrite(PIN_TX_LPF_A, 0);
    digitalWrite(PIN_TX_LPF_B, 0);
    digitalWrite(PIN_TX_LPF_C, 1);
  }
}

/**
 * This is the most frequently called function that configures the
 * radio to a particular frequeny, sideband and sets up the transmit filters
 *
 * The transmit filter relays are powered up only during the tx so they dont
 * draw any current during rx.
 *
 * The carrier oscillator of the detector/modulator is permanently fixed at
 * uppper sideband. The sideband selection is done by placing the second oscillator
 * either 12 Mhz below or above the 45 Mhz signal thereby inverting the sidebands
 * through mixing of the second local oscillator.
 */

void setFrequency(uint32_t f)
{
  setTXFilters(f);

  /*
    if (isUSB){
      si5351bx_setfreq(2, firstIF  + f);
      si5351bx_setfreq(1, firstIF + usbCarrier);
    }
    else{
      si5351bx_setfreq(2, firstIF + f);
      si5351bx_setfreq(1, firstIF - usbCarrier);
    }
  */
  // alternative to reduce the intermod spur
  if (settings.isUSB)
  {
    si5351bx_setfreq(2, firstIF + f);
    si5351bx_setfreq(1, firstIF + usbCarrier);
  }
  else
  {
    si5351bx_setfreq(2, firstIF + f);
    si5351bx_setfreq(1, firstIF - usbCarrier);
  }

  settings.frequency = f;
}

/**
 * startTx is called by the PTT, cw keyer and CAT protocol to
 * put the uBitx in tx mode. It takes care of rit settings, sideband settings
 * Note: In cw mode, doesnt key the radio, only puts it in tx mode
 * CW offest is calculated as lower than the operating frequency when in LSB mode, and vice versa in USB mode
 */

void startTx(uint8_t txMode)
{
  digitalWrite(PIN_TX_RX, 1);
  settings.inTx = 1;

  if (settings.ritOn)
  {
    // save the current as the rx frequency
    ritRxFrequency = settings.frequency;
    setFrequency(ritTxFrequency);
  }
  else
  {
    if (settings.splitOn)
    {
      if (settings.vfoActive == VFO_B)
      {
        settings.vfoActive = VFO_A;
        settings.isUSB = isUsbVfoA;
        settings.frequency = settings.vfoA;
      }
      else if (settings.vfoActive == VFO_A)
      {
        settings.vfoActive = VFO_B;
        settings.frequency = settings.vfoB;
        settings.isUSB = isUsbVfoB;
      }
    }
    setFrequency(settings.frequency);
  }

  if (txMode == TX_CW)
  {
    // turn off the second local oscillator and the bfo
    si5351bx_setfreq(0, 0);
    si5351bx_setfreq(1, 0);

    // shif the first oscillator to the tx frequency directly
    // the key up and key down will toggle the carrier unbalancing
    // the exact cw frequency is the tuned frequency + sidetone
    if (settings.isUSB)
      si5351bx_setfreq(2, settings.frequency + settings.sideTone);
    else
      si5351bx_setfreq(2, settings.frequency - settings.sideTone);
  }
  updateDisplay();
}

void stopTx()
{
  settings.inTx = false;

  digitalWrite(PIN_TX_RX, LOW);    // turn off the tx
  si5351bx_setfreq(0, usbCarrier); // set back the cardrier oscillator anyway, cw tx switches it off

  if (settings.ritOn) {
    setFrequency(ritRxFrequency);
  }
  else
  {
    if (settings.splitOn)
    {
      // vfo Change
      if (settings.vfoActive == VFO_B)
      {
        settings.vfoActive = VFO_A;
        settings.frequency = settings.vfoA;
        settings.isUSB = isUsbVfoA;
      }
      else if (settings.vfoActive == VFO_A)
      {
        settings.vfoActive = VFO_B;
        settings.frequency = settings.vfoB;
        settings.isUSB = isUsbVfoB;
      }
    }
    setFrequency(settings.frequency);
  }
  updateDisplay();
}

/**
 * ritEnable is called with a frequency parameter that determines
 * what the tx frequency will be
 */
void ritEnable(uint32_t f)
{
  settings.ritOn = 1;
  // save the non-rit frequency back into the VFO memory
  // as RIT is a temporary shift, this is not saved to EEPROM
  ritTxFrequency = f;
}

// this is called by the RIT menu routine
void ritDisable()
{
  if (settings.ritOn)
  {
    settings.ritOn = 0;
    setFrequency(ritTxFrequency);
    updateDisplay();
  }
}

/**
 * Basic User Interface Routines. These check the front panel for any activity
 */

/**
 * The PTT is checked only if we are not already in a cw transmit session
 * If the PTT is pressed, we shift to the ritbase if the rit was on
 * flip the T/R line to T and update the display to denote transmission
 */

void checkPTT()
{
  // we don't check for ptt when transmitting cw
  if (settings.cwTimeout > 0)
    return;

  if ( (digitalRead(PIN_PTT) == LOW) && !settings.inTx)
  {
    startTx(TX_SSB);
    active_delay(50); // debounce the PTT
  }

  if ((digitalRead(PIN_PTT) == HIGH) && settings.inTx)
    stopTx();
}

static void checkButton()
{
  // only if the button is pressed
  if (!btnDown())
    return;
  active_delay(50);
  if (!btnDown()) // debounce
    return;

  doMenu();
  // wait for the button to go up again
  while (btnDown())
    active_delay(10);
  active_delay(50); // debounce
}

/**
 * The tuning jumps by 50 Hz on each step when you tune slowly
 * As you spin the encoder faster, the jump size also increases
 * This way, you can quickly move to another band by just spinning the
 * tuning knob
 */

void doTuning()
{
  int s = enc_read();
  
  if (s != 0)
  {
    uint32_t prev_freq = settings.frequency;

    if (s > 4)
      settings.frequency += 10000l;
    else if (s > 2)
      settings.frequency += 500;
    else if (s > 0)
      settings.frequency += 50l;
    else if (s > -2)
      settings.frequency -= 50l;
    else if (s > -4)
      settings.frequency -= 500l;
    else
      settings.frequency -= 10000l;

    // check if we have crossed the 10 MHz border
    if (prev_freq < 10000000l && settings.frequency > 10000000l)
      settings.isUSB = true;

    // check if we have crossed the 10 MHz border
    if (prev_freq > 10000000l && settings.frequency < 10000000l)
      settings.isUSB = false;

    setFrequency(settings.frequency);
    updateDisplay();
  }
}

/**
 * RIT only steps back and forth by 100 hz at a time
 */
void doRIT()
{
  int knob = enc_read();
  uint32_t old_freq = settings.frequency;

  if (knob < 0)
    settings.frequency -= 100l;
  else if (knob > 0)
    settings.frequency += 100;

  if (old_freq != settings.frequency)
  {
    setFrequency(settings.frequency);
    updateDisplay();
  }
}

/**
 * The settings are read from EEPROM. The first time around, the values may not be
 * present or out of range, in this case, some intelligent defaults are copied into the
 * variables.
 */
void initSettings()
{
  uint8_t x;
  // read the settings from the eeprom and restore them
  // if the readings are off, then set defaults
  EEPROM.get(MASTER_CAL, calibration);
  EEPROM.get(USB_CAL, usbCarrier);
  EEPROM.get(VFO_A, settings.vfoA);
  EEPROM.get(VFO_B, settings.vfoB);
  EEPROM.get(CW_SIDETONE, settings.sideTone);
  EEPROM.get(CW_SPEED, settings.cwSpeed);

  if (usbCarrier > 11060000l || usbCarrier < 11048000l)
    usbCarrier = 11052000l;
  if (settings.vfoA > 35000000l || 3500000l > settings.vfoA)
    settings.vfoA = 7150000l;
  if (settings.vfoB > 35000000l || 3500000l > settings.vfoB)
    settings.vfoB = 14150000l;
  if (settings.sideTone < 100 || 2000 < settings.sideTone)
    settings.sideTone = 800;
  if (settings.cwSpeed < 10 || 1000 < settings.cwSpeed)
    settings.cwSpeed = 100;

  /*
   * The VFO modes are read in as either 2 (USB) or 3(LSB), 0, the default
   * is taken as 'uninitialized
   */

  EEPROM.get(VFO_A_MODE, x);

  switch (x)
  {
  case VFO_MODE_USB:
    isUsbVfoA = 1;
    break;
  case VFO_MODE_LSB:
    isUsbVfoA = 0;
    break;
  default:
    if (settings.vfoA > 10000000l)
      isUsbVfoA = 1;
    else
      isUsbVfoA = 0;
  }

  EEPROM.get(VFO_B_MODE, x);
  switch (x)
  {
  case VFO_MODE_USB:
    isUsbVfoB = 1;
    break;
  case VFO_MODE_LSB:
    isUsbVfoB = 0;
    break;
  default:
    if (settings.vfoA > 10000000l)
      isUsbVfoB = 1;
    else
      isUsbVfoB = 0;
  }

  // set the current mode
  settings.isUSB = isUsbVfoA;

  /*
   * The keyer type splits into two variables
   */
  EEPROM.get(CW_KEY_TYPE, x);

  if (x == 0)
    Iambic_Key = false;
  else if (x == 1)
  {
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  }
  else if (x == 2)
  {
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }
}

void initPorts()
{

  analogReference(DEFAULT);

  //??
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_FBUTTON, INPUT_PULLUP);

  // configure the function button to use the external pull-up
  //  pinMode(FBUTTON, INPUT);
  //  digitalWrite(FBUTTON, HIGH);

  pinMode(PIN_PTT, INPUT_PULLUP);
  pinMode(PIN_ANALOG_KEYER, INPUT_PULLUP);

  pinMode(PIN_CW_TONE, OUTPUT);
  digitalWrite(PIN_CW_TONE, 0);

  pinMode(PIN_TX_RX, OUTPUT);
  digitalWrite(PIN_TX_RX, 0);

  pinMode(PIN_TX_LPF_A, OUTPUT);
  pinMode(PIN_TX_LPF_B, OUTPUT);
  pinMode(PIN_TX_LPF_C, OUTPUT);
  digitalWrite(PIN_TX_LPF_A, 0);
  digitalWrite(PIN_TX_LPF_B, 0);
  digitalWrite(PIN_TX_LPF_C, 0);

  pinMode(PIN_CW_KEY, OUTPUT);
  digitalWrite(PIN_CW_KEY, 0);
}

void setup()
{
  settings.vfoA = 7150000L;
  settings.vfoB = 14200000L;
  settings.ritOn = false;
  settings.splitOn = false;

  settings.cwSpeed = 100; // this is actuall the dot period in milliseconds
  settings.cwTimeout = 0; // milliseconds to go before the cw transmit line is released and the radio goes back to rx mode

  settings.inTx = false;    // it is set to 1 if in transmit mode (whatever the reason : cw, ptt or cat)
  settings.keyDown = false; // in cw mode, denotes the carrier is being transmitted
  settings.isUSB = false;   // upper sideband was selected, this is reset to the default for the

  settings.cwDelayTime = 60;

  settings.txCAT = false;
  settings.sideTone = 800;
  settings.vfoActive = VFO_A;
  settings.meter_reading = 0;

  Serial.begin(38400);
  Serial.flush();

  initDisplay();

  // we print this line so this shows up even if the raduino
  // crashes later in the code
  printLine2("uBITX v5.11");
  // active_delay(500);

  //  initMeter(); //not used in this build
  initSettings();
  initPorts();
  initOscillators();

  settings.frequency = settings.vfoA;
  setFrequency(settings.vfoA);
  updateDisplay();

  if (btnDown())
  {
    factory_alignment();
  }
}

/**
 * The loop checks for keydown, ptt, function button and tuning.
 */

uint8_t flasher = 0;
void loop()
{

  cwKeyer();
  if (!settings.txCAT)
    checkPTT();
  checkButton();

  // tune only when not tranmsitting
  if (!settings.inTx)
  {
    if (settings.ritOn)
      doRIT();
    else
      doTuning();
  }

  // we check CAT after the encoder as it might put the radio into TX
  checkCAT();
}
