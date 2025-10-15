#ifndef _GROUPING_H
#define _GROUPING_H

#include <Arduino.h>
#include "debug.hpp"
#include "Bar.hpp"


// There are 4 bars of leds in a group at max.
// A bar can have up to 16 leds.

#define MAX_NR_BARS 4
#define RANDOM_TICK

class Grouping {
    private:
        byte groupNr;
        byte *ledNumbersInThisGroupVector;
        byte algoritm;
        byte nrOfLedsInThisBar;
        byte nrOfBarsInThisGroup;
        byte brightness;
        byte speed;
        Bar *bars[MAX_NR_BARS];
        byte *algoritmPattern;
        int *initialLedPattern;
        bool anyBarChanges = false; // true if there are any changes in the bars of this grouping.
        byte *ledNumbersInThisGroupMatrix[4];
        byte tickCnt = 0;
    public:

        Grouping() {}

        Grouping(byte _groupNr,
                 byte _nrOfLedsInThisBar,
                 byte _nrOfBarsInThisGroup,
                 byte _brightness,
                 byte _speed,
                 byte *_algoritmPattern,
                 int *_initialLedPattern):
                    groupNr(_groupNr),
                    nrOfLedsInThisBar(_nrOfLedsInThisBar),
                    nrOfBarsInThisGroup(_nrOfBarsInThisGroup),
                    brightness(_brightness),
                    speed(_speed),
                    algoritmPattern(_algoritmPattern),
                    initialLedPattern(_initialLedPattern) {
                    for (byte barNr = 0; barNr < nrOfBarsInThisGroup; barNr++) {
                        ledNumbersInThisGroupMatrix[barNr] = new byte[nrOfLedsInThisBar];
                        getLedNrsForThisBar(groupNr, barNr, nrOfLedsInThisBar, ledNumbersInThisGroupMatrix[barNr]);
                        bars[barNr] = new Bar((byte) barNr, nrOfLedsInThisBar, ledNumbersInThisGroupMatrix[barNr],
                                                     brightness, algoritmPattern, initialLedPattern[barNr]);
                    }
            }

        ~Grouping() {
            for (byte barNr = 0; barNr < nrOfBarsInThisGroup; barNr++) {
                free(ledNumbersInThisGroupMatrix[barNr]);
                delete bars[barNr];
            }
        }

        void deleteBar(byte barNr) {
            delete bars[barNr];
            free(ledNumbersInThisGroupMatrix[barNr]);
        }

        void newBar(byte barNr) {
            deleteBar(barNr);
            getLedNrsForThisBar(groupNr, barNr, nrOfLedsInThisBar, ledNumbersInThisGroupVector);
            // Choose at random among the algoritms and initialPatterns.
            bars[barNr] = new Bar((byte) barNr, nrOfLedsInThisBar, ledNumbersInThisGroupVector,
                                    brightness, algoritmPattern, initialLedPattern[barNr]);
        }

        void getLedNrsForThisBar(byte groupNr, byte barNr, byte nrOfLedsInThisBar, byte *ledNumbersInThisGroup) {
            getLedNrsPerBar(groupNr, barNr, nrOfLedsInThisBar, ledNumbersInThisGroup);
        }

        bool tick() {
            anyBarChanges = false;
            byte nrCalls = algoritmPattern[1];
            if ((tickCnt % speed) == 0) {
                for (int barNr = 0; barNr < nrOfBarsInThisGroup; barNr++) {
                    for (byte callNr = 0; callNr < nrCalls; callNr++) {
                        #ifdef RANDOM_TICK
                            if (randomNumberGenerator.getRandomNumber(0, 101, 7) < RANDOM_TICK_PERCENTAGE) {
                                anyBarChanges |= bars[barNr]->tick();
                            }
                        #else
                            anyBarChanges |= bars[barNr]->tick();
                        #endif
                    }
                }
            }
            tickCnt++;
            return(anyBarChanges);
        }

        #define NR_OF_REPETITIONS 5

        void flashOnOff() {
            for (int rep = 0; rep < NR_OF_REPETITIONS; rep++) {
                // set all leds of this group
                for (byte barNr = 0; barNr < nrOfBarsInThisGroup; barNr++) {
                    bars[barNr]->allOn();
                }
                display->show();
                delay(100);
                // reset all leds
                for (byte barNr = 0; barNr < nrOfBarsInThisGroup; barNr++) {
                    bars[barNr]->allOff();
                }
                display->show();
                delay(100);
            }
        }

        void show() {
            if (anyBarChanges) {
                for (int barNr = 0; barNr < nrOfBarsInThisGroup; barNr++) {
                    bars[barNr]->show();
                }
                anyBarChanges = false;
            }
        }

        void test() {
            for (int barNr = 0; barNr < nrOfBarsInThisGroup; barNr++) {
                bars[barNr]->test();
            }
        }
};

#endif