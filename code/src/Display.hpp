#ifndef _DISPLAY
#define _DISPLAY

#include "LedController.hpp"

#define NR_OF_LEDS 192

class Display: public LedController {

    private:
        byte ledState[NR_OF_LEDS]; // There are 192 leds which can be ON or OFF.
        byte ledBrightness[NR_OF_DEVICES];
        bool ledStateChanged = false;
        bool brightnessChanged = false;

    public:

        Display(int dataInPin, int clockPin, int loadPin, int _nrOfLedMatrices = 3):
            LedController(dataInPin, clockPin, loadPin, _nrOfLedMatrices) {
                init();
            }

        void setLedBrightness(int deviceNr, int brightnessValue) {
            ledBrightness[deviceNr] = brightnessValue;
            brightnessChanged = true;
            // Now set brightness of actual device.
            setBrightness(deviceNr, brightnessValue);
        }


        void setAllBrightnessValues(byte value) {
            for (byte deviceNr = 0; deviceNr < NR_OF_DEVICES; deviceNr++) {
                setLedBrightness(deviceNr, value);
            }
            brightnessChanged = true;
        }

        void setAllBrightnessValues() {
            for (byte deviceNr = 0; deviceNr < NR_OF_DEVICES; deviceNr++) {
                setLedBrightness(deviceNr, ledBrightness[deviceNr]);
            }
            brightnessChanged = true;
        }

        bool getState() {
            if (ledStateChanged) {
                ledStateChanged = false;
                return(true);
            } else {
                return(false);
            }
        }

        void setLed(byte ledNr) {
            if (ledState[ledNr] == 0) {
                ledState[ledNr] = 1;
                ledStateChanged = true;
            }
        }

        void resetLed(byte ledNr) {
            // leds are counted from 1 to and including NR_OF_LEDS
            if (ledState[ledNr] == 1) {
                ledState[ledNr] = 0;
                ledStateChanged = true;
            }
        }

        void show() {
            DRC drc;
            for (byte ledNr = 0; ledNr < NR_OF_LEDS; ledNr++) {
                drc = ledToRowColumn(ledNr);
                if (ledState[ledNr]) {
                    setByte(drc);
                } else {
                    resetByte(drc);
                }
            }
        }

        void show(bool anyChanges) {
            if (anyChanges) {
                show();
            }
        }

        void clear() {
            for (byte x = 0; x < 8; x++) {
                for (byte y = 0; y < 8; y++) {
                    resetByte(0, x, y);
                }
            }
            for (byte ledNr = 0; ledNr < NR_OF_LEDS; ledNr++) {
                ledState[ledNr] = 0;
            }
            ledStateChanged = true;
        }

        void setRow(byte deviceNr, byte row, byte value) {
            debug_print("Not implemented yet.");
        }

        void setColumn(byte deviceNr, byte column, byte value) {
            debug_print("Not implemented yet.");
        }

        void tick() {
            if (brightnessChanged) {
                // Set brightness values of MAX devices and refresh all leds.
                setAllBrightnessValues();
                brightnessChanged = false;
            }
            if (ledStateChanged) {
                show();
                ledStateChanged = false;
            }
        }

        void init() {
            debug_print("Display::init()\n");
            clear();
            ledStateChanged = true;
            for (byte deviceNr = 0; deviceNr < NR_OF_DEVICES; deviceNr++) {
                /* Set the brightness to a medium values */
                setLedBrightness(deviceNr, MAX_BRIGHTNESS);
                /* and clear the display part for this device. */
                clearDisplay(deviceNr);
                // Make sure the MAX chip does not go to sleep.
                shutdown(deviceNr, false);
            }
            brightnessChanged = true;
        }
};

#endif