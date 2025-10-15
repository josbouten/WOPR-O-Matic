#ifndef _LEDCONTROLLER
#define _LEDCONTROLLER
#include "LedControl.h"
#include "Led.hpp"
#include "debug.hpp"


class LedController: public LedControl {

    private:
        int nrOfLedMatrices;

    public:

        LedController() = default;

        /*
        pin 12 is connected to the DataIn
        pin 11 is connected to the CLK
        pin 10 is connected to LOAD
        nr = nr of MAX72XX.
        */

        void init() {
            debug_print("LedController::init()\n");
            for (int i = 0; i < nrOfLedMatrices; i++) {
                shutdown(i, false);
                // medium value brightness
                setIntensity(i, 1);
                clearDisplay(i);
            }
        }

        LedController(int dataInPin = 12, int clockPin = 11, int loadPin = 10, int _nrOfLedMatrices = 3):
            LedControl(dataInPin, clockPin, loadPin, _nrOfLedMatrices) {
                nrOfLedMatrices = _nrOfLedMatrices;
                init();
            }

        void setBrightness(int deviceNr, int brightness) {
            setIntensity(deviceNr, brightness);
        }

        // Led(0, 0) should be the left most led.
        // We need to swap the dot column to be the first ledcolumn.

        // Set byte in MAX7219, effectively turning an LED on.
        void setByte(DRC drc) {
            setLed(drc.deviceNr, drc.row, drc.column, true);
        }

        // Set byte in MAX7219, effectively turning an LED on.
        void setByte(int deviceNr, int row, int column) {
            if (column < 7) {
                column++;
            } else {
                column = 0;
            }
            setLed(deviceNr, row, column, true);
        }

        // Reset byte in MAX7219, effectively turning an LED off.
        void resetByte(DRC drc) {
            setLed(drc.deviceNr, drc.row, drc.column, false);
        }

        // Reset byte in MAX7219, effectively turning an LED off.
        void resetByte(int deviceNr, int row, int column) {
            if (column < 7) {
                column++;
            } else {
                column = 0;
            }
            setLed(deviceNr, row, column, false);
        }
};

#endif