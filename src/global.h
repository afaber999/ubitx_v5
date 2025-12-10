#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
// ============================================================================
// Function Prototypes - uBITX v5
// Extracted from all .ino source files
// ============================================================================

// ============================================================================
// ubitx_v5.1_code.ino
// ============================================================================
void active_delay(int delay_by);
void setTXFilters(unsigned long freq);
void setTXFilters_v5(unsigned long freq);
void setFrequency(unsigned long f);
void startTx(uint8_t txMode);
void stopTx();
void ritEnable(unsigned long f);
void ritDisable();
void checkPTT();
void checkButton();
void doTuning();
void doRIT();
void initSettings();
void initPorts();
void loop();

// ============================================================================
// ubitx_ui.ino
// ============================================================================
int btnDown();
void initMeter();
void drawMeter(int8_t needle);
void printLine(char linenmbr, char *c);
void printLine1(char *c);
void printLine2(char *c);
void updateDisplay();
uint8_t enc_state(void);
int enc_read(void);

// ============================================================================
// ubitx_si5351.ino
// ============================================================================
void i2cWrite(uint8_t reg, uint8_t val);
void i2cWriten(uint8_t reg, uint8_t *vals, uint8_t vcnt);
void si5351bx_init();
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void si5351_set_calibration(int32_t cal);
void initOscillators();

// ============================================================================
// ubitx_menu.ino
// ============================================================================
int getValueByKnob(int minimum, int maximum, int step_size, int initial, char *prefix, char *postfix);
int menuBand(int btn);
void menuRitToggle(int btn);
void menuVfoToggle(int btn);
void menuSidebandToggle(int btn);
void menuSplitToggle(int btn);
int menuCWSpeed(int btn);
void menuExit(int btn);
int menuSetup(int btn);
int calibrateClock();
int menuSetupCalibration(int btn);
void printCarrierFreq(unsigned long freq);
void menuSetupCarrier(int btn);
void menuSetupCwTone(int btn);
void menuSetupCwDelay(int btn);
void menuSetupKeyer(int btn);
void menuReadADC(int btn);
void doMenu();

// ============================================================================
// ubitx_keyer.ino
// ============================================================================
uint8_t getPaddle();
void cwKeydown();
void cwKeyUp();
char update_PaddleLatch(uint8_t isUpdateKeyState);
void cwKeyer(void);

// ============================================================================
// ubitx_factory_alignment.ino
// ============================================================================
void btnWaitForClick();
void factory_alignment();

// ============================================================================
// ubitx_cat.ino
// ============================================================================
uint8_t setHighNibble(uint8_t b, uint8_t v);
uint8_t setLowNibble(uint8_t b, uint8_t v);
uint8_t getHighNibble(uint8_t b);
uint8_t getLowNibble(uint8_t b);
void getDecimalDigits(unsigned long number, uint8_t *result, int digits);
void writeFreq(unsigned long freq, uint8_t *cmd);
unsigned long readFreq(uint8_t *cmd);
void processCATCommand2(uint8_t *cmd);
void checkCAT();

#endif // GLOBAL_H
