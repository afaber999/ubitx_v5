/** Menus
 *  The Radio menus are accessed by tapping on the function button.
 *  - The main loop() constantly looks for a button press and calls doMenu() when it detects
 *  a function button press.
 *  - As the encoder is rotated, at every 10th pulse, the next or the previous menu
 *  item is displayed. Each menu item is controlled by it's own function.
 *  - Eache menu function may be called to display itself
 *  - Each of these menu routines is called with a button parameter.
 *  - The btn flag denotes if the menu itme was clicked on or not.
 *  - If the menu item is clicked on, then it is selected,
 *  - If the menu item is NOT clicked on, then the menu's prompt is to be displayed
 */

/** A generic control to read variable values
 */

#include "global.h"
#include <EEPROM.h>

static uint8_t menuOn = 0;        // set to 1 when the menu is being displayed, if a menu item sets it to zero, the menu is exited
static bool modeCalibrate = true; // this mode of menus shows extended menus to calibrate the oscillators and choose the proper

static void menuSetupCalibration(bool btn);
static void menuBand(bool btn);
static int getValueByKnob(int minimum, int maximum, int step_size, int initial, const char *prefix, const char *postfix);
static void menuCWSpeed(bool btn);
static void menuReadADC(bool btn);
static void menuSetupKeyer(bool btn);
static void menuSetupCwDelay(bool btn);
static void menuSetupCwTone(bool btn);
static void printCarrierFreq(uint32_t freq);
static void menuSplitToggle(bool btn);
static void menuRitToggle(bool btn);
static void menuExit(bool btn);
static void menuSidebandToggle(bool btn);

void waitForBtnUp()
{
  while (btnDown())
  {
    active_delay(50);
  }
  active_delay(50);
}

static int getValueByKnob(int minimum, int maximum, int step_size, int initial, const char *prefix, const char *postfix)
{
  int knob = 0;
  int knob_value;

  while (btnDown())
  {
    active_delay(100);
  }

  active_delay(200);
  knob_value = initial;

  strcpy(bBuf, prefix);
  itoa(knob_value, cBuf, 10);
  strcat(bBuf, cBuf);
  strcat(bBuf, postfix);
  printLine2(bBuf);
  active_delay(300);

  while (!btnDown() && !pttOn())
  {

    knob = enc_read();
    if (knob != 0)
    {
      if (knob_value > minimum && knob < 0)
        knob_value -= step_size;
      if (knob_value < maximum && knob > 0)
        knob_value += step_size;

      printLine2(prefix);
      itoa(knob_value, cBuf, 10);
      strcpy(bBuf, cBuf);
      strcat(bBuf, postfix);
      printLine1(bBuf);
    }
    checkCAT();
  }

  return knob_value;
}

// # Menu: 1

static void menuBand(bool btn)
{
  int knob = 0;

  // band = frequency/1000000l;
  // offset = frequency % 1000000l;

  if (!btn)
  {
    printLine2("Band Select    \x7E");
    return;
  }

  printLine2("Band Select:");
  // wait for the button menu select button to be lifted)
  waitForBtnUp();

  ritDisable();

  while (!btnDown())
  {

    knob = enc_read();
    if (knob != 0)
    {
      /*
      if (band > 3 && knob < 0)
        band--;
      if (band < 30 && knob > 0)
        band++;
      if (band > 10)
        settings.isUSB = true;
      else
        settings.isUSB = false;
      setFrequency(((uint32_t)band * 1000000l) + offset); */
      if (knob < 0 && settings.frequency > 3000000l)
        setFrequency(settings.frequency - 200000l);
      if (knob > 0 && settings.frequency < 30000000l)
        setFrequency(settings.frequency + 200000l);
      if (settings.frequency > 10000000l)
        settings.isUSB = true;
      else
        settings.isUSB = false;
      updateDisplay();
    }
    checkCAT();
    active_delay(20);
  }

  waitForBtnUp();

  printLine2("");
  updateDisplay();
  menuOn = 0;
}

// Menu #2
static void menuRitToggle(bool btn)
{
  if (!btn)
  {
    if (settings.ritOn)
      printLine2("RIT On \x7E Off");
    else
      printLine2("RIT Off \x7E On");
  }
  else
  {
    if (!settings.ritOn)
    {
      // enable RIT so the current frequency is used at transmit
      ritEnable(settings.frequency);
      printLine2("RIT is On");
    }
    else
    {
      ritDisable();
      printLine2("RIT is Off");
    }
    menuOn = 0;
    active_delay(500);
    printLine2("");
    updateDisplay();
  }
}

// Menu #3
void menuVfoToggle(bool btn)
{

  if (!btn)
  {
    if (settings.vfoActive == VFO_A)
      printLine2("VFO A \x7E B");
    else
      printLine2("VFO B \x7E A");
  }
  else
  {
    if (settings.vfoActive == VFO_B)
    {
      settings.vfoB = settings.frequency;
      isUsbVfoB = settings.isUSB;
      EEPROM.put(VFO_B, settings.frequency);
      if (isUsbVfoB)
        EEPROM.put(VFO_B_MODE, VFO_MODE_USB);
      else
        EEPROM.put(VFO_B_MODE, VFO_MODE_LSB);

      settings.vfoActive = VFO_A;
      //      printLine2("Selected VFO A  ");
      settings.frequency = settings.vfoA;
      settings.isUSB = isUsbVfoA;
    }
    else
    {
      settings.vfoA = settings.frequency;
      isUsbVfoA = settings.isUSB;
      EEPROM.put(VFO_A, settings.frequency);
      if (isUsbVfoA)
        EEPROM.put(VFO_A_MODE, VFO_MODE_USB);
      else
        EEPROM.put(VFO_A_MODE, VFO_MODE_LSB);

      settings.vfoActive = VFO_B;
      //      printLine2("Selected VFO B  ");
      settings.frequency = settings.vfoB;
      settings.isUSB = isUsbVfoB;
    }

    ritDisable();
    setFrequency(settings.frequency);
    updateDisplay();
    printLine2("");
    // exit the menu
    menuOn = 0;
  }
}

// Menu #4
static void menuSidebandToggle(bool btn)
{
  if (!btn)
  {
    if (settings.isUSB)
      printLine2("USB \x7E LSB");
    else
      printLine2("LSB \x7E USB");
  }
  else
  {
    if (settings.isUSB)
    {
      settings.isUSB = false;
      printLine2("LSB Selected");
      active_delay(500);
      printLine2("");
    }
    else
    {
      settings.isUSB = true;
      printLine2("USB Selected");
      active_delay(500);
      printLine2("");
    }
    // Added by KD8CEC
    if (settings.vfoActive == VFO_B)
    {
      isUsbVfoB = settings.isUSB;
    }
    else
    {
      isUsbVfoB = settings.isUSB;
    }
    updateDisplay();
    menuOn = 0;
  }
}

// Split communication using VFOA and VFOB by KD8CEC
// Menu #5
static void menuSplitToggle(bool btn)
{
  if (!btn)
  {
    if (!settings.splitOn)
      printLine2("Split Off \x7E On");
    else
      printLine2("Split On \x7E Off");
  }
  else
  {
    if (settings.splitOn)
    {
      settings.splitOn = false;
      printLine2("Split ON");
    }
    else
    {
      settings.splitOn = true;
      settings.ritOn = false;
      printLine2("Split Off");
    }
    active_delay(500);
    printLine2("");
    updateDisplay();
    menuOn = 0;
  }
}

static void menuCWSpeed(bool btn)
{
  int wpm;

  wpm = 1200 / settings.cwSpeed;

  if (!btn)
  {
    strcpy(bBuf, "CW: ");
    itoa(wpm, cBuf, 10);
    strcat(bBuf, cBuf);
    strcat(bBuf, " WPM     \x7E");
    printLine2(bBuf);
    return;
  }

  wpm = getValueByKnob(1, 100, 1, wpm, "CW: ", " WPM>");

  printLine2("CW Speed set!");
  settings.cwSpeed = 1200 / wpm;
  EEPROM.put(CW_SPEED, settings.cwSpeed);
  active_delay(500);

  printLine2("");
  updateDisplay();
  menuOn = 0;
}

static void menuExit(bool btn)
{
  if (!btn)
  {
    printLine2("Exit Menu      \x7E");
  }
  else
  {
    printLine2("Exiting...");
    active_delay(500);
    printLine2("");
    updateDisplay();
    menuOn = 0;
  }
}

/**
 * The calibration routines are not normally shown in the menu as they are rarely used
 * They can be enabled by choosing this menu option
 */
int menuSetup(bool btn)
{
  if (!btn)
  {
    if (!modeCalibrate)
      printLine2("Settings       \x7E");
    else
      printLine2("Settings \x7E Off");
  }
  else
  {
    if (!modeCalibrate)
    {
      modeCalibrate = true;
      printLine2("Settings On");
    }
    else
    {
      modeCalibrate = false;
      printLine2("Settings Off");
    }

    waitForBtnUp();
    printLine2("");
    return 10;
  }
  return 0;
}

// this is used by the si5351 routines in the ubitx_5351 file
extern uint32_t si5351bx_vcoa;

void calibrateClock()
{
  int knob = 0;

  // keep clear of any previous button press
  waitForBtnUp();

  digitalWrite(PIN_TX_LPF_A, 0);
  digitalWrite(PIN_TX_LPF_B, 0);
  digitalWrite(PIN_TX_LPF_C, 0);

  settings.pllCalibration = 0;

  settings.isUSB = true;

  // turn off the second local oscillator and the bfo
  si5351_set_calibration(settings.pllCalibration);
  startTx(TX_CW);
  si5351bx_setfreq(2, 10000000l);

  strcpy(bBuf, "#1 10 MHz cal:");
  ltoa(settings.pllCalibration / 8750, cBuf, 10);
  strcat(bBuf, cBuf);
  printLine2(bBuf);

  while (!btnDown())
  {

    if (pttOn() && !settings.keyDown)
      cwKeydown();
    if (!pttOn() && settings.keyDown)
      cwKeyUp();

    knob = enc_read();

    if (knob > 0)
      settings.pllCalibration += 875;
    else if (knob < 0)
      settings.pllCalibration -= 875;
    else
      continue; // don't update the frequency or the display

    si5351_set_calibration(settings.pllCalibration);
    si5351bx_setfreq(2, 10000000l);
    strcpy(bBuf, "#1 10 MHz cal:");
    ltoa(settings.pllCalibration / 8750, cBuf, 10);
    strcat(bBuf, cBuf);
    printLine2(bBuf);
  }

  settings.cwTimeout = 0;
  settings.keyDown = 0;
  stopTx();

  printLine2("Calibration set!");
  EEPROM.put(MASTER_CAL, settings.pllCalibration);
  initOscillators(settings.pllCalibration);
  setFrequency(settings.frequency);
  updateDisplay();

  waitForBtnUp();
}

static void menuSetupCalibration(bool btn)
{
  if (!btn)
  {
    printLine2("Setup:Calibrate\x7E");
    return;
  }

  printLine1("Press PTT & tune");
  printLine2("to exactly 10 MHz");
  active_delay(2000);
  calibrateClock();
}

static void printCarrierFreq(uint32_t freq)
{

  memset(cBuf, 0, sizeof(cBuf));
  memset(bBuf, 0, sizeof(bBuf));

  ultoa(freq, bBuf, DEC);

  strncat(cBuf, bBuf, 2);
  strcat(cBuf, ".");
  strncat(cBuf, &bBuf[2], 3);
  strcat(cBuf, ".");
  strncat(cBuf, &bBuf[5], 1);
  printLine2(cBuf);
}

void menuSetupCarrier(bool btn)
{
  int knob = 0;

  if (!btn)
  {
    printLine2("Setup:BFO      \x7E");
    return;
  }

  printLine1("Tune to best Signal");
  printLine2("Press to confirm. ");
  active_delay(1000);

  usbCarrier = 11053000l;
  si5351bx_setfreq(0, usbCarrier);
  printCarrierFreq(usbCarrier);

  // disable all clock 1 and clock 2
  while (!btnDown())
  {
    knob = enc_read();

    if (knob > 0)
      usbCarrier -= 50;
    else if (knob < 0)
      usbCarrier += 50;
    else
      continue; // don't update the frequency or the display

    si5351bx_setfreq(0, usbCarrier);
    printCarrierFreq(usbCarrier);

    active_delay(100);
  }

  printLine2("Carrier set!    ");
  EEPROM.put(USB_CAL, usbCarrier);
  active_delay(1000);

  si5351bx_setfreq(0, usbCarrier);
  setFrequency(settings.frequency);
  updateDisplay();
  printLine2("");
  menuOn = 0;
}

static void menuSetupCwTone(bool btn)
{
  int knob = 0;
  int prev_sideTone;

  if (!btn)
  {
    printLine2("Setup:CW Tone  \x7E");
    return;
  }

  prev_sideTone = settings.sideTone;
  printLine1("Tune CW tone");
  printLine2("PTT to confirm. ");
  active_delay(1000);
  tone(PIN_CW_TONE, settings.sideTone);

  // disable all clock 1 and clock 2
  while (!pttOn() && !btnDown())
  {
    knob = enc_read();

    if (knob > 0 && settings.sideTone < 2000)
      settings.sideTone += 10;
    else if (knob < 0 && settings.sideTone > 100)
      settings.sideTone -= 10;
    else
      continue; // don't update the frequency or the display

    tone(PIN_CW_TONE, settings.sideTone);
    itoa(settings.sideTone, bBuf, 10);
    printLine2(bBuf);

    checkCAT();
    active_delay(20);
  }
  noTone(PIN_CW_TONE);
  // save the setting
  if (pttOn())
  {
    printLine2("Sidetone set!    ");
    EEPROM.put(CW_SIDETONE, settings.sideTone);
    active_delay(2000);
  }
  else
    settings.sideTone = prev_sideTone;

  printLine2("");
  updateDisplay();
  menuOn = 0;
}

static void menuSetupCwDelay(bool btn)
{
  if (!btn)
  {
    printLine2("Setup:CW Delay \x7E");
    return;
  }

  active_delay(500);
  settings.cwDelayTime = getValueByKnob(10, 1000, 50, settings.cwDelayTime, "CW Delay>", " msec");

  printLine1("CW Delay Set!");
  printLine2("");
  active_delay(500);
  updateDisplay();
  menuOn = 0;
}

static void menuSetupKeyer(bool btn)
{
  int tmp_key, knob;

  if (!btn)
  {
    if (!Iambic_Key)
      printLine2("Setup:CW(Hand)\x7E");
    else if (keyerControl & IAMBICB)
      printLine2("Setup:CW(IambA)\x7E");
    else
      printLine2("Setup:CW(IambB)\x7E");
    return;
  }

  active_delay(500);

  if (!Iambic_Key)
    tmp_key = 0; // hand key
  else if (keyerControl & IAMBICB)
    tmp_key = 2; // Iambic B
  else
    tmp_key = 1;

  while (!btnDown())
  {
    knob = enc_read();
    if (knob < 0 && tmp_key > 0)
      tmp_key--;
    if (knob > 0)
      tmp_key++;

    if (tmp_key > 2)
      tmp_key = 0;

    if (tmp_key == 0)
      printLine1("Hand Key?");
    else if (tmp_key == 1)
      printLine1("Iambic A?");
    else if (tmp_key == 2)
      printLine1("Iambic B?");
  }

  active_delay(500);
  if (tmp_key == 0)
    Iambic_Key = false;
  else if (tmp_key == 1)
  {
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  }
  else if (tmp_key == 2)
  {
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }

  EEPROM.put(CW_KEY_TYPE, tmp_key);

  printLine1("Keyer Set!");
  active_delay(600);
  printLine1("");

  // Added KD8CEC
  printLine2("");
  updateDisplay();
  menuOn = 0;
}

static void menuReadADC(bool btn)
{
  int adc;

  if (!btn)
  {
    printLine2("6:Setup>Read ADC>");
    return;
  }
  delay(500);

  while (!btnDown())
  {
    adc = analogRead(PIN_ANALOG_KEYER);
    itoa(adc, bBuf, 10);
    printLine1(bBuf);
  }

  printLine1("");
  updateDisplay();
}

void doMenu()
{
  int select = 0;

  waitForBtnUp();

  menuOn = 2;

  while (menuOn)
  {
    int i = enc_read();
    bool btnState = btnDown();

    if (i > 0)
    {
      if (modeCalibrate && select + i < 150)
        select += i;
      if (!modeCalibrate && select + i < 80)
        select += i;
    }
    else
    {
      select += i; // caught ya, i is already -ve here, so you add it
      if (select < 0)
      {
        select = 0;
      }
    }

    if (select < 10)
      menuBand(btnState);
    else if (select < 20)
      menuRitToggle(btnState);
    else if (select < 30)
      menuVfoToggle(btnState);
    else if (select < 40)
      menuSidebandToggle(btnState);
    else if (select < 50)
      menuSplitToggle(btnState);
    else if (select < 60)
      menuCWSpeed(btnState);
    else if (select < 70)
      select += menuSetup(btnState);
    else if (select < 80 && !modeCalibrate)
      menuExit(btnState);
    else if (select < 90 && modeCalibrate)
      menuSetupCalibration(btnState); // crystal
    else if (select < 100 && modeCalibrate)
      menuSetupCarrier(btnState); // lsb
    else if (select < 110 && modeCalibrate)
      menuSetupCwTone(btnState);
    else if (select < 120 && modeCalibrate)
      menuSetupCwDelay(btnState);
    else if (select < 130 && modeCalibrate)
      menuReadADC(btnState);
    else if (select < 140 && modeCalibrate)
      menuSetupKeyer(btnState);
    else
      menuExit(btnState);
  }

  waitForBtnUp();
  checkCAT();
}
