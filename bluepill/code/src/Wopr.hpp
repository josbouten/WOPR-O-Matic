#ifndef _WOPR_H
#define _WOPR_H

#include "Grouping.hpp"

class Wopr {

    #define NR_OF_GROUPINGS 10

    private:
        bool testing;
        Grouping *grouping[NR_OF_GROUPINGS];

        // Each algoritm vector consists of a series of tuples of 3 numbers.
        // tuple element 1: algoritm type
        // tuple element 2: nr of steps to make per tick
        // tuple element 3: to wrap around or not to wrap aroung
        byte algoritmA[12] = { 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1 }; // 0
        byte algoritmB[12] = { 7, 1, 1, 7, 1, 1, 6, 1, 1, 6, 1, 1 }; // 1
        byte algoritmC[12] = { 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1 }; // 2
        byte algoritmD[ 6] = { 4, 2, 1, 4, 1, 1 }; // 3
        byte algoritmE[ 6] = { 2, 1, 1, 2, 1, 1 }; // 4
        byte algoritmF[ 6] = { 1, 1, 1, 1, 1, 1 }; // 5
        byte algoritmG[ 6] = { 0, 1, 1, 1, 1, 1 }; // 6
        byte algoritmH[ 3] = { 8, 2, 1 };          // 7
        byte algoritmI[ 3] = { 1, 2, 1 };          // 8
        byte algoritmJ[ 6] = { 1, 2, 1, 0, 1, 1 }; // 9

        byte *algo1[2] = { algoritmH, algoritmI };
        byte *algo2[5] = { algoritmD, algoritmE, algoritmF, algoritmG, algoritmJ };
        byte *algo4[3] = { algoritmA, algoritmB, algoritmC };

        byte *initialAlgoritm[NR_OF_GROUPINGS];
        /*  vul initialAlgoritm met 10 algoritmes als volgt:
            kies voor pos 0, 1, 2 random 3 groepen uit algo4,
            kies voor pos 3, 4, 5, en 6 random 4 groepen uit algo2,
            kies voor pos 7 en 8 random 2 groepen uit algo1,
            kies voor pos 9 random 1 groep uit algo2.
        */

        // Which leds are ON and which are OFF at startup?
        int  groupA[4] = { 0x0001, 0x0003, 0x0007, 0x000E }; // 0
        int  groupB[4] = { 0x0E0E, 0xA001, 0xF1FF, 0x0A0A }; // 1
        int  groupC[4] = { 0x0001, 0x0005, 0x0007, 0x0003 }; // 2
        int  groupD[2] = { 0x0001, 0x0002 }; // 3
        int  groupE[2] = { 0xF001, 0xF001 }; // 4
        int  groupF[2] = { 0x0001, 0x0004 }; // 5
        int  groupG[2] = { 0x0008, 0x0001 }; // 6
        int  groupH[1] = { 0x1FFE };         // 7
        int  groupI[1] = { 0xFE3F };         // 8
        int  groupJ[2] = { 0x0001, 0x0008 }; // 9
        int  groupTest =  0xFFFF;

        // There are groups that have 1 bar, 2 bars or 4 bars.
        // Each contains the appropriate startup values.
        int *group1[2] = { groupH, groupI };
        int *group2[5] = { groupD, groupE, groupF, groupG, groupJ };
        int *group4[3] = { groupA, groupB, groupC };

        int *initialLedPattern[NR_OF_GROUPINGS];
        bool anyGroupChanges = false;

        /*
        vul initialLedPattern met 10 groepen als volgt:
        kies voor pos 0, 1, 2 random 3 groepen uit group4,
        kies voor pos 3, 4, 5, en 6 random 4 groepen uit group2,
        kies voor pos 7 en 8 random 2 groepen uit group1,
        kies voor pos 9 random 1 groep uit group2.
        */
       #define SLOW 3
       #define FAST 1
       // Speed of pattern change. Short patterns change more slowly than long ones so
       // that they appear to change at the same pace.
       byte speed[NR_OF_GROUPINGS] = { SLOW, FAST, SLOW, SLOW, FAST, SLOW, SLOW, FAST, FAST, SLOW };

       void init(byte groupNr) {
            switch(groupNr) {
                case 0:
                case 1:
                case 2:
                    initialLedPattern[groupNr] = group4[randomNumberGenerator.getRandomNumber(0, 3, 4)];
                    initialAlgoritm[groupNr] = algo4[randomNumberGenerator.getRandomNumber(0, 3, 4)];
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                    initialLedPattern[groupNr] = group2[randomNumberGenerator.getRandomNumber(0, 5, 4)];
                    initialAlgoritm[groupNr] = algo2[randomNumberGenerator.getRandomNumber(0, 5, 4)];
                    break;
                case 7:
                case 8:
                    initialLedPattern[groupNr] = group1[randomNumberGenerator.getRandomNumber(0, 2, 4)];
                    initialAlgoritm[groupNr] = algo1[randomNumberGenerator.getRandomNumber(0, 2, 4)];
                    break;
                case 9:
                    initialLedPattern[groupNr] = group2[randomNumberGenerator.getRandomNumber(0, 5, 4)];
                    initialAlgoritm[groupNr] = algo2[randomNumberGenerator.getRandomNumber(0, 5, 4)];
                    break;
                default:
                    debug_print2("Wopr::init: this should not happen! groupNr = %d\n", groupNr);
            }
       }

    public:
        Wopr(bool _testing = false): testing(_testing) {
            debug_print("WOPR-O-Matic constructor begin.\n");

            if (testing) {
                for (int groupNr = 0; groupNr < NR_OF_GROUPINGS; groupNr++) {
                    // Choose initial led pattern and algoritm for this group.
                    init(groupNr);
                    debug_print2("group: %d\n", groupNr);
                    // Create group.
                    createTestGroup(groupNr);
                }
            } else {
                for (int groupNr = 0; groupNr < NR_OF_GROUPINGS; groupNr++) {
                    // Choose initial led pattern and algoritm for this group.
                    init(groupNr);
                    debug_print2("group: %d\n", groupNr);
                    // Create group.
                    createGroup(groupNr);
                }
            }
            debug_print("\nWOPR-O-Matic constructor end.\n");
        }

        ~Wopr() {
            debug_print("WOPR-O-Matic destructor begin.\n");
            for (int groupNr = 0; groupNr < NR_OF_GROUPINGS; groupNr++) {
                delete grouping[groupNr];
            }
            debug_print("WOPR-O-Matic destructor end.\n");
        }

        void createGroup(byte groupNr) {
            byte nrOfLedsInThisBar = allLeds[groupNr][0];
            byte nrOfBars = allLeds[groupNr][1];
            grouping[groupNr] = new Grouping(groupNr,
                                             nrOfLedsInThisBar,
                                             nrOfBars,
                                             MIN_BRIGHTNESS,
                                             speed[groupNr],
                                             initialAlgoritm[groupNr],
                                             initialLedPattern[groupNr]);
        }


        void createTestGroup(byte groupNr) {
            byte nrOfLedsInThisBar = allLeds[groupNr][0];
            byte nrOfBars = allLeds[groupNr][1];
            grouping[groupNr] = new Grouping(groupNr,
                                             nrOfLedsInThisBar,
                                             nrOfBars,
                                             MIN_BRIGHTNESS,
                                             speed[groupNr],
                                             initialAlgoritm[groupNr],
                                             initialLedPattern[groupNr]);
            // For test purposes we set all leds to 1;
            initialLedPattern[groupNr] = &groupTest;
        }

        void replaceGroup(byte groupNr) {
            delete grouping[groupNr];
            // Choose led pattern and algoritm at random.
            init(groupNr);
            // Create the led group (and bars in the group).
            createGroup(groupNr);
        }

        void showVersion(byte versionLSB, byte versionMSB) {
            // flash one of the leds corresponding with the version number.
            grouping[versionLSB]->flashOnOff();
            grouping[versionMSB]->flashOnOff();
        }

        void test() {
            for (int groupNr = 0; groupNr < NR_OF_GROUPINGS; groupNr++) {
                grouping[groupNr]->test();
            }
        }

        bool tick() {
            anyGroupChanges = false;
            for (int groupNr = 0; groupNr < NR_OF_GROUPINGS; groupNr++) {
                //debug_print2("wopr-ticking group: %d\n", groupNr);
                anyGroupChanges |= grouping[groupNr]->tick();
            }
            return(anyGroupChanges);
        }

        void show() {
            for (int groupNr = 0; groupNr < NR_OF_GROUPINGS; groupNr++) {
                grouping[groupNr]->show();
            }
            anyGroupChanges = false;
        }
};

#endif