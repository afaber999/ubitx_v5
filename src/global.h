#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// Function Prototypes - uBITX v5
// Extracted from all .ino source files
// ============================================================================

// ============================================================================
// ubitx_v5.1_code.ino
// ============================================================================
void active_delay(uint32_t delay_by);
void setTXFilters(uint32_t freq);
void setTXFilters_v5(uint32_t freq);
void setFrequency(uint32_t f);
void startTx(uint8_t txMode);
void stopTx();
void ritEnable(uint32_t f);
void ritDisable();
void checkPTT();
void doTuning();
void doRIT();
void initSettings();
void initPorts();
void loop();

// ============================================================================
// ubitx_ui.ino
// ============================================================================
void initDisplay();
int btnDown();
void initMeter();
void drawMeter(int8_t needle);
void printLine(int linenmbr, const char *c);
void printLine1(const char *c);
void printLine2(const char *c);
void updateDisplay();
uint8_t enc_state(void);
int enc_read(void);

// ============================================================================
// ubitx_si5351.ino
// ============================================================================
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void si5351_set_calibration(int32_t cal);
void initOscillators();

// ============================================================================
// ubitx_menu.ino
// ============================================================================
void calibrateClock();

void menuRitToggle(int btn);
void menuVfoToggle(int btn);
void menuSidebandToggle(int btn);
void menuExit(int btn);
int menuSetup(int btn);
void menuSetupCarrier(int btn);
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
void getDecimalDigits(uint32_t number, uint8_t *result, int digits);
void writeFreq(uint32_t freq, uint8_t *cmd);
uint32_t readFreq(uint8_t *cmd);
void processCATCommand2(uint8_t *cmd);
void checkCAT();

// we directly generate the CW by programmin the Si5351 to the cw tx frequency, hence, both are different modes
// these are the parameter passed to startTx
#define TX_SSB 0
#define TX_CW 1
#define IAMBICB 0x10 // 0 for Iambic A, 1 for Iambic B

/***********************************************************************************************************************
 * These are the indices where these user changable settinngs are stored  in the EEPROM
 */
#define MASTER_CAL 0
#define LSB_CAL 4
#define USB_CAL 8
#define SIDE_TONE 12
// these are ids of the vfos as well as their offset into the eeprom storage, don't change these 'magic' values
#define VFO_A 16
#define VFO_B 20
#define CW_SIDETONE 24
#define CW_SPEED 28

// These are defines for the new features back-ported from KD8CEC's software
// these start from beyond 256 as Ian, KD8CEC has kept the first 256 uint8_ts free for the base version
#define VFO_A_MODE 256 // 2: LSB, 3: USB
#define VFO_B_MODE 257

// values that are stroed for the VFO modes
#define VFO_MODE_LSB 2
#define VFO_MODE_USB 3

// handkey, iambic a, iambic b : 0,1,2f
#define CW_KEY_TYPE 358

/***********************************************************************************************************************
 * EEPROM END
 */

typedef struct
{
    uint32_t vfoA;
    uint32_t vfoB;
    bool ritOn;
    int cwSpeed;
    bool splitOn;       // working split, uses VFO B as the transmit frequency, (NOT IMPLEMENTED YET)
    uint32_t cwTimeout; // milliseconds to go before the cw transmit line is released and the radio goes back to rx mode
    uint8_t cwDelayTime;

    bool inTx;    // it is set to 1 if in transmit mode (whatever the reason : cw, ptt or cat)
    bool keyDown; // in cw mode, denotes the carrier is being transmitted
    bool isUSB;   // upper sideband was selected, this is reset to the default for the
    bool txCAT;   // turned on if the transmitting due to a CAT command

    uint32_t frequency;
    uint32_t sideTone;
    char vfoActive;

} settings_t;

extern settings_t settings;
extern char cBuf[30];
extern char bBuf[30];
extern char printBuff[2][31]; // mirrors what is showing on the two lines of the display
extern uint32_t usbCarrier;
extern int32_t calibration;
extern uint8_t keyerControl;
extern bool Iambic_Key;

extern char isUsbVfoA;
extern char isUsbVfoB;

#endif // GLOBAL_H
