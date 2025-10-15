#ifndef __BAR_H

    #define __BAR_H
    #include <Arduino.h>

    #include "debug.hpp"
    #include "BitUtils.hpp"
    #include "Display.hpp"
    #include "LfsrRandomNumberGenerator.hpp"

    // There are 16 leds in a bar at max.
    #define MAX_NR_OF_LEDS 16
    #define ON 1
    #define OFF 0
    #define NO_WRAP false
    #define WRAP true

    bool randomBool(void);
    extern Display *display;
    extern LFSR_RandomNumberGenerator randomNumberGenerator;

    class Bar {
        private:
            byte barNr;
            byte nrOfLedsInThisBar;
            byte *ledNumbersInThisBar;// Vector of led numbers.
            byte brightness;
            byte *algoritmPattern;    // On/Off pattern for LEDs.
            int initialLedPattern;    // Initial On/Off settings.
            byte *ledState;           // State of a led can be On or OFF.
            byte functionNr;
            byte index;
            byte shiftLeftCounter;
            byte shiftRightCounter;
            byte shiftCounter;
            byte intervalCounter;
            byte swapIndex;
            byte cnt;
            bool anyLedChanges;

        public:

            Bar(){}

            Bar(byte _barNr, byte _nrOfLedsInThisBar, byte *_ledNumbersInThisBar, byte _brightness, byte *_algoritmPattern, int _initialLedPattern):
                barNr(_barNr),
                nrOfLedsInThisBar(_nrOfLedsInThisBar),
                ledNumbersInThisBar(_ledNumbersInThisBar),
                brightness(_brightness),
                algoritmPattern(_algoritmPattern),
                initialLedPattern(_initialLedPattern) {
                    // Allocate memory to store the state of each LED in this bar.
                    ledState = new byte[nrOfLedsInThisBar];
                    shiftLeftCounter = 0;
                    shiftRightCounter = 0;
                    shiftCounter = 0;
                    intervalCounter = 0;
                    swapIndex = 0;
                    cnt = 0;
                    initLedState();
                    index = barNr * 3;
                    functionNr = algoritmPattern[index];
                    anyLedChanges = false;
            }

            ~Bar() {
                delete ledState;
            }

            void initLedState() {
                // Filling the initial led state should be done using the algoritm for this bar.
                switch(functionNr) {
                    case 0:
                    case 1:
                    case 4:
                        for (byte nr = 0; nr < nrOfLedsInThisBar; nr++) {
                            // The leds are numbered in mirror image to the patterns; therefor
                            // we must cross index them.
                            ledState[nrOfLedsInThisBar - 1 - nr] = bitRead(initialLedPattern, nr);
                        }
                    break;
                    case 2:
                    case 3:
                        for (byte nr = 0; nr < nrOfLedsInThisBar; nr++) {
                            // The algoritme will fill the LED bar later, so we start with all LEDs dimmed.
                            ledState[nr] = 0;
                        }
                    default:
                    break;
                }

            }

            void shiftRight(byte *ledState, byte nrOfLedsInThisBar, bool wrap = true) {
                byte savedState = ledState[nrOfLedsInThisBar - 1];
                for (byte i = nrOfLedsInThisBar - 1; i > 0; i--) {
                    ledState[i] = ledState[i - 1];
                }
                if (wrap) {
                    ledState[0] = savedState;
                }
            }

            void shiftRight(bool wrap = true) {
                byte savedState = ledState[nrOfLedsInThisBar - 1];
                for (byte i = nrOfLedsInThisBar - 1; i > 0; i--) {
                    ledState[i] = ledState[i - 1];
                }
                if (wrap) {
                    ledState[0] = savedState;
                }
            }

            void allOff() {
                for (int x = 0; x < nrOfLedsInThisBar; x++) {
                    display->resetLed(ledNumbersInThisBar[x]);
                }
            }

            void allOn() {
                for (int x = 0; x < nrOfLedsInThisBar; x++) {
                    debug_print3("%02d -> %03d\t", x, ledNumbersInThisBar[x]);
                    display->setLed(ledNumbersInThisBar[x]);
                }
                debug_print("\n");
            }

            void test() {
                allOn();
                display->show(true);
                delay(50);
                allOff();
                delay(50);
            }


            void shiftLeft(bool wrap = true) {
                byte savedState = ledState[0];
                for (byte i = 0; i < nrOfLedsInThisBar - 1; i++) {
                    ledState[i] = ledState[i + 1];
                }
                if (wrap) {
                    ledState[nrOfLedsInThisBar - 1] = savedState;
                }
            }

            void generateAndShiftLeft(bool wrap, float fraction) {
                // The fraction of the nrOfLedsInThisBar determines how many shift steps are made.
                // If this number is > 1 then the leds are shifted more steps than the pattern length.
                if (shiftLeftCounter < fraction * nrOfLedsInThisBar - 1) {
                    // random new led or not
                    // shift left until the end, then stop.
                    if (randomNumberGenerator.getRandomNumber(0, 101, 7) < 70) {
                        ledState[nrOfLedsInThisBar - 1] = 1; // Put lit LED in bar.
                    } else {
                        ledState[nrOfLedsInThisBar - 1] = 0; // Put dark LED in bar.
                    }
                    shiftLeft(NO_WRAP);
                    shiftLeftCounter++;
                    // Erase from right to center.
                } else {
                    intervalCounter++;
                    if (intervalCounter > 10) {
                        intervalCounter = 0;
                        shiftLeftCounter = 0;
                        shiftCounter = 0;
                    }
                }
            }

            void generateAndShiftRight(bool wrap, float fraction) {
                // The fraction of the nrOfLedsInThisBar determines how many shift steps are made.
                // If this number is > 1 then the leds are shifted more stpes than the pattern length.
                if (shiftRightCounter < fraction * nrOfLedsInThisBar  - 1) {
                    // random new led or noy
                    // shift right until the end, then stop.
                    if (randomNumberGenerator.getRandomNumber(0, 101, 7) < 70) {
                        ledState[0] = 1; // Put lit LED in bar.
                    } else {
                        ledState[0] = 0; // Put dark LED in bar.
                    }
                    shiftRight(NO_WRAP);
                    shiftRightCounter++;
                    // Erase from right to center.
                } else {
                    intervalCounter++;
                    if (intervalCounter > 10) {
                        intervalCounter = 0;
                        shiftRightCounter = 0;
                        shiftCounter = 0;
                    }
                }
            }

            bool tickThisBar() {
                anyLedChanges = false;
                bool wrap = algoritmPattern[index + 2] == 0 ? false: true;
                #ifdef RANDOM_DIRECTION_CHANGE
                    // Sometimes change direction of shift.
                    if (randomNumberGenerator.getRandomNumber(0, 101, 7) < RANDOM_DIRECTION_CHANGE_PERCENTAGE) {
                        if (functionNr == 1) {
                            functionNr = 0;
                        } else {
                            functionNr = 1;
                        }
                        algoritmPattern[index] = functionNr;
                    }
                #endif
                // Advance the algoritm one step
                switch(functionNr) {
                    case 0:
                        shiftLeft(wrap);
                        anyLedChanges = true;
                    break;
                    case 1:
                        shiftRight(wrap);
                        anyLedChanges = true;
                    break;
                    case 2:
                        // Erase 4/5 from right to left.
                        if (shiftCounter < (4 * nrOfLedsInThisBar / 5 - 1)) {
                            ledState[shiftCounter] = 0;
                            shiftCounter++;
                            anyLedChanges = true;
                        } else {
                            if ((cnt % 2) == 0) {
                                // Generate and shift pattern to the left.
                                generateAndShiftLeft(wrap, 1.5);
                                anyLedChanges = true;
                            }
                            cnt++;
                        }
                        break;
                    case 3:
                        // Erase 4/5 from left to right
                        if (shiftCounter < (4 * nrOfLedsInThisBar / 5 - 1)) {
                            ledState[nrOfLedsInThisBar - shiftCounter - 1] = 0;
                            shiftCounter++;
                            anyLedChanges = true;
                        } else {
                            if ((cnt % 2) == 0) {
                                // Generate and shift pattern to the right.
                                generateAndShiftRight(wrap, 1.25);
                                anyLedChanges = true;
                            }
                            cnt++;
                        }
                        break;
                    case 4:
                        // Swap left and right from outside to inside.
                        std::swap(ledState[swapIndex], ledState[nrOfLedsInThisBar - 1 - swapIndex]);
                        anyLedChanges = true;
                        swapIndex++;
                        if (swapIndex > nrOfLedsInThisBar / 2) {
                            swapIndex = 0;
                        }
                        break;
                    case 5: // Move to the left, move to the right and repeat.
                        if (cnt < nrOfLedsInThisBar / 2) {
                            shiftLeft(true);
                        } else {
                            shiftRight(true);
                        }
                        anyLedChanges = true;
                        cnt++;
                        if (cnt > nrOfLedsInThisBar - 1) {
                            cnt = 0;
                        }
                    case 6:
                        // Erase all from right to left.
                        if (shiftCounter < nrOfLedsInThisBar) {
                            ledState[shiftCounter] = 0;
                            shiftCounter++;
                        } else {
                            if ((cnt % 2) == 0) {
                                // Generate and shift pattern to the left 1.5 lengths.
                                generateAndShiftLeft(wrap, 1.5);
                            }
                            cnt++;
                        }
                        anyLedChanges = true;
                        break;
                    case 7:
                        // Erase from left to right
                        if (shiftCounter < nrOfLedsInThisBar) {
                            ledState[nrOfLedsInThisBar - shiftCounter - 1] = 0;
                            shiftCounter++;
                        } else {
                            if ((cnt % 2) == 0) {
                                // Generate and shift pattern to the right 2 lengths.
                                generateAndShiftRight(wrap, 2);
                            }
                            cnt++;
                        }
                        anyLedChanges = true;
                        break;
                    case 8:
                        // Random leds on and off
                        cnt = randomNumberGenerator.getRandomNumber(0, nrOfLedsInThisBar, 4);
                        ledState[cnt] = randomNumberGenerator.getRandomNumber(0, 2, 2) > 0 ? 0: 1;
                        anyLedChanges = true;
                    default:
                    break;
                }
                return(anyLedChanges);
            }

            bool tick() {
                #ifdef RANDOM_TICK
                if (randomNumberGenerator.getRandomNumber(0, 101, 7) < RANDOM_TICK_PERCENTAGE) {
                    return(tickThisBar());
                } else {
                    return(false);
                }
                #else
                    return(tickThisBar());
                #endif
            }

            void show() {
                DRC drc;
                if (anyLedChanges) {
                    // Set or reset lednr ledsInThisBar[i] according
                    // to the state in ledState[i].
                    // This will result in an actual LED to be lit or
                    // not via the MAX chips.
                    for (byte i = 0; i < nrOfLedsInThisBar; i++) {
                        drc = ledToRowColumn(ledNumbersInThisBar[i]);
                        if (ledState[i] > 0) {
                            display->setByte(drc);
                        } else {
                            display->resetByte(drc);
                        }
                    }
                    anyLedChanges = false;
                }
            }
    };

#endif