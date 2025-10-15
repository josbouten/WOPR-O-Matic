#ifndef __DEBUG_H
    #define __DEBUG_H

    //#define AVR_BOARD // for AVR-based arduino boards
    #define ARM_BOARD // for ARM based boards

    #ifdef __arm__
        // should use uinstd.h to define sbrk but Due causes a conflict
        extern "C" char* sbrk(int incr);
    #else  // __ARM__
        extern char *__brkval;
    #endif  // __arm__

    #ifdef CHECK_MEMORY
        int freeMemory() {
            char top;
            #ifdef __arm__
                return &top - reinterpret_cast<char*>(sbrk(0));
            #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
                return &top - __brkval;
            #else  // __arm__
                return __brkval ? &top - __brkval : &top - __malloc_heap_start;
            #endif  // __arm__
        }
    #endif

    #ifdef DEBUG
        #include "LibPrintf.h"
        #define debug_begin(z) Serial.begin(z)
        #define debug_print(z) printf(z)
        #define debug_print2(z, y) printf(z, y)
        #define debug_print3(z, y, x) printf(z, y, x)
        #define debug_print4(z, y, x, w) printf(z, y, x, w)
        #define debug_print5(z, y, x, w, v) printf(z, y, x, w, v)
        #define debug_print6(z, y, x, w, v, u) printf(z, y, x, w, v, u)
        #define debug_print8(z, y, x, w, v, u, t, s) printf(z, y, x, w, v, u, t, s)
        #define debug_delay(z) delay(z)
    #else
        #define debug_begin(x)
        #define debug_print(x)
        #define debug_print2(z, y)
        #define debug_print3(z, y, x)
        #define debug_print4(z, u, x, w)
        #define debug_print5(z, y, x, w, v)
        #define debug_print6(z, y, x, w, v, u)
        #define debug_print8(z, y, x, w, v, u, t, s)
        #define debug_delay(z)
    #endif

#endif