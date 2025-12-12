#include "global.h"

static bool simPtt = false;
static bool simBtn = false;
static int simEncReadVal = 0;

bool simulateIO = 1;

int simEncRead()
{
    int ret = simEncReadVal;
    simEncReadVal = 0;
    return ret;
}

static int enc_prev_state = 3;

/**
 * The A7 And A6 are purely analog lines on the Arduino Nano
 * These need to be pulled up externally using two 10 K resistors
 *
 * There are excellent pages on the Internet about how these encoders work
 * and how they should be used. We have elected to use the simplest way
 * to use these encoders without the complexity of interrupts etc to
 * keep it understandable.
 *
 * The enc_state returns a two-bit number such that each bit reflects the current
 * value of each of the two phases of the encoder
 *
 * The enc_read returns the number of net pulses counted over 50 msecs.
 * If the puluses are -ve, they were anti-clockwise, if they are +ve, the
 * were in the clockwise directions. Higher the pulses, greater the speed
 * at which the enccoder was spun
 */

static uint8_t enc_state(void)
{
    HandleSimIo();
    return (analogRead(PIN_ENC_A) > 500 ? 1 : 0) + (analogRead(PIN_ENC_B) > 500 ? 2 : 0);
}

int enc_read(void)
{
    if (simulateIO)
    {
        HandleSimIo();
        return simEncRead();
    }

    int result = 0;
    uint8_t newState;
    int enc_speed = 0;

    uint32_t stop_by = millis() + 50;

    while (millis() < stop_by)
    {                           // check if the previous state was stable
        newState = enc_state(); // Get current state

        if (newState != enc_prev_state)
            delay(1);

        if (enc_state() != newState || newState == enc_prev_state)
            continue;
        // these transitions point to the encoder being rotated anti-clockwise
        if ((enc_prev_state == 0 && newState == 2) ||
            (enc_prev_state == 2 && newState == 3) ||
            (enc_prev_state == 3 && newState == 1) ||
            (enc_prev_state == 1 && newState == 0))
        {
            result--;
        }
        // these transitions point o the enccoder being rotated clockwise
        if ((enc_prev_state == 0 && newState == 1) ||
            (enc_prev_state == 1 && newState == 3) ||
            (enc_prev_state == 3 && newState == 2) ||
            (enc_prev_state == 2 && newState == 0))
        {
            result++;
        }
        enc_prev_state = newState; // Record state for next pulse interpretation
        enc_speed++;
        active_delay(1);
    }
    return (result);
}

bool pttOn()
{
    HandleSimIo();
    if (simulateIO)
        return simPtt;
    return digitalRead(PIN_PTT) == LOW;
}

// returns true if the button is pressed
bool btnDown()
{
    HandleSimIo();
    if (simulateIO)
        return simBtn;

    return digitalRead(PIN_FBUTTON) == LOW;
}

void print_freq(uint32_t freq)
{
}

int loops = 0;
void HandleSimIo()
{
    delay(1);
    if ((loops++ % 1000) == 0)
        Serial.print("HANDLE!!!\n");
    if (Serial.available())
    {
        char c = Serial.read();
        Serial.print("SimIo:");
        Serial.println(c);
        switch (c)
        {
        case 'T':
        case 't':
            Serial.write("Calibration: ");
            Serial.println(settings.pllCalibration);
            Serial.write("usbCarrier: ");
            Serial.println(usbCarrier);
            break;
        case '1':
            simEncReadVal = -2;
            break;
        case '2':
            simEncReadVal = -1;
            break;
        case '3':
            simEncReadVal = 1;
            break;
        case '4':
            simEncReadVal = +2;
            break;
        case 'P':
            simPtt = true;
            break;
        case 'p':
            simPtt = false;
            break;
        case 'b':
            simBtn = false;
            break;
        case 'B':
            simBtn = true;
            break;
        }
    }
}