#ifndef _LED_HPP
#define _LED_HPP

#include <Arduino.h>
#include "debug.hpp"

// Element 0 of each group is the number of columns of leds in that group.
// Element 1 of echt group is the number of rows of leds in that group.
// The next numbers are tuples of 4 that contain the led number of the first led of each column.
// E.g. byte groupB[] = { 16, 4,   4,  12,  20,  28,  64,  72,  80,  88, 68, 76, 84, 92, 128, 136, 144, 152 };
// exists of 16 columns of 4 leds. The indices to the lednumbers in those culumns are
// 4, 12, 20, 28 for the first column,
// and 64, 72, 80, 88 for the 2nd column etc.


byte groupA[] = {  4, 4,   0,   8,  16,  24 };
byte groupB[] = { 16, 4,   4,  64,  68, 128, 12,  72,  76, 136,  20, 80, 84, 144, 28, 88, 92, 152 };
byte groupC[] = {  4, 4, 132, 140, 148, 156 };
byte groupD[] = {  4, 2,  32,  40 };
byte groupE[] = { 16, 2,  36,  96,  100, 160, 44, 104, 108, 168 };
byte groupF[] = {  4, 2, 164, 172 };
byte groupG[] = {  4, 2,  48,  56 };
byte groupH[] = { 16, 1,  52, 112, 116, 176 };
byte groupI[] = { 16, 1,  60, 120, 124, 184 };
byte groupJ[] = {  4, 2, 180, 188 };

#define NR_OF_GROUPS 10

byte *allLeds[NR_OF_GROUPS] = { groupA, groupB, groupC, groupD, groupE, groupF, groupG, groupH, groupI, groupJ };

void printPerBar(byte lower, byte upper, byte groupNr){
    byte blockSize = 4;
    for (byte index = lower; index < upper; index++) {
        for (byte j = 0; j < blockSize; j++) {
            debug_print2("%3d\t", allLeds[groupNr][index] + j);
        }
    }
    debug_print("\n");
}

void printPerGroup(byte groupNr) {
    int nrOfLeds = allLeds[groupNr][0];
    int nrOfBars = allLeds[groupNr][1];
    byte offset = 2;
    for (byte barNr = 0; barNr < nrOfBars; barNr++) {
        byte lower = nrOfLeds / 4 * barNr + offset;
        byte upper = nrOfLeds / 4 * (barNr + 1) + offset;
        printPerBar(lower, upper, groupNr);
    }
}

void getLedNrsPerBar(byte groupNr, byte barNr, byte nrOfLedsInThisBar, byte *ledsInThisBar) {
    byte offset = 2;
    byte lower = nrOfLedsInThisBar / 4 * barNr + offset;
    byte upper = nrOfLedsInThisBar / 4 * (barNr + 1) + offset;
    debug_print2("bar: %2d:\t", barNr);
    byte blockSize = 4;
    byte i = 0;
    for (byte index = lower; index < upper; index++) {
        for (byte j = 0; j < blockSize; j++) {
            ledsInThisBar[i++] = allLeds[groupNr][index] + j;
        }
    }
    for (int i = 0; i < nrOfLedsInThisBar; i++) {
        debug_print2("%2d\t", ledsInThisBar[i]);
    }
    debug_print("EOB \n");
}

//
// print de leds per bar om te zie of ze de goede led nummers hebben.
//

void printLedIndices() {
    // Per group van leds worden tuples van indices naar de lednummers geprint.
    // M.a.w. print de groupA...groupJ rijen
    for (byte groupNr = 0; groupNr < NR_OF_GROUPS; groupNr++) {
        printPerGroup(groupNr);
    }
}

void getLedNrs(byte groupNr, byte barNr, byte nrOfLedsInThisBar, byte ledsInGroup[]) {
    // Per group van leds worden tuples van indices naar de lednummers geprint.
    // M.a.w. print de groupA...groupJ rijen
    byte offset = 2;
    byte lower = allLeds[groupNr][barNr + offset];
    byte upper = lower + nrOfLedsInThisBar;
    debug_print6("g: %2d\tb: %2d\t#nrOfLedsInThisBar: %2d\tlower: %2d\t upper: %2d", groupNr, barNr, nrOfLedsInThisBar, lower, upper);
    for (byte index = lower, i = 0; index < upper; index++, i++) {
        // Store the ledNrs.
        ledsInGroup[i] = lower + i;
        debug_print3("lednr[%d]=%3d\t", i, ledsInGroup[i]);
    }
    //debug_print("\n");
}

byte divRemain(byte numerator, byte denominator) {
    byte deeltal = numerator / denominator;
    return( numerator - (deeltal * denominator));
}

byte getLedNr(byte groupNr, byte x, byte y) {
    byte M = allLeds[groupNr][0];
    byte tupleNr, index, ledNr = 255;
    byte N = allLeds[groupNr][1];
    if ((x < M) && (y < N)) {
        tupleNr = x / 4;
        index = 2 + tupleNr * N + y;
        ledNr = allLeds[groupNr][index] + divRemain(x, 4) - 1;
        return(ledNr);
    } else {
        return(255);
    }
}

struct DRC {
    byte deviceNr;
    byte row;
    byte column;
};

DRC ledToRowColumn(byte ledNr) {
    // Return deviceNr and row and column of led for
    // the MAX7912 devices.
    byte index[8] = {1, 2, 3, 4, 5, 6, 7, 0};
    byte deviceNr = ledNr / 64;
    ledNr = ledNr % 64;
    byte row = ledNr / 8;
    byte column = ledNr - row * 8;
    column = index[column];
    DRC result = {deviceNr, row, column};
    return(result);
}

#endif