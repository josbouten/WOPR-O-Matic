//#define MAKE_VIDEO // In case we want to make a video e.g. for youtube we need to dimm the LEDs as much as possible.

#define DEBUG // Uncomment this if you want to see some debug info on the serial port. This may slow down the shift of patterns a bit.
//#define TESTING // Uncomment this if you continuously want to check all leds are working correctly.

// Author: J.S. Bouten, 2023, 2024
// You can see the WOPR in action here: https://www.youtube.com/watch?v=L203UwEdGJY

// For hardware demo videos got to https://www.youtube.com/@zaphodb5361
// For videos of my music go to https://www.youtube.com/@josbouten

#define VERSION_LSB 0
#define VERSION_MSB 2 // i.e. 0.2

#define DYNAMIC_BRIGHTNESS                    // LED brightness will depend on the amount of day light.
#define GROUP_REFRESH                         // WOPR will show new patterns for a random group on a regular basis.
#define RANDOM_DIRECTION_CHANGE               // Patterns will change direction by chance.
#define RANDOM_DIRECTION_CHANGE_PERCENTAGE 02
#define RANDOM_TICK                           // Bars will be ticked at random
#define RANDOM_TICK_PERCENTAGE 95

#include "debug.hpp"
#include <Arduino.h>

// Pin definitions for connecting to an STM32 BluePill.
#define MAX7219_DIN_0_PIN PB15 // MOSI, 5 V tolerant pin, HW SPI
#define MAX7219_LOAD_PIN  PB14 // HW SPI
#define MAX7219_CLOCK_PIN PB13 // CLK, HW SPI
#define EXT_CLOCK_IN      PB12 // Extern clock is connected to 5V tolerant pin. It is meant to be able to trigger an interrupt.
#define LIGHT_SENSOR      PA0  // LDR + resistor divider

#define NR_OF_DEVICES  3

#define MIN_BRIGHTNESS 0 // max brightness of LEDs
#define MAX_BRIGHTNESS 4 // max brightness of LEDs

#include "Led.hpp"
#include "Display.hpp"
Display *display;

#include "LfsrRandomNumberGenerator.hpp"

// We use a floating input to seed the random number generator.
LFSR_RandomNumberGenerator randomNumberGenerator(analogRead(PA1));

#include "Wopr.hpp"
#define INITIAL_CPU_SPEED     1
#define INITIAL_REFRESH_RATE 10
Wopr *wopr;

#define OFF 0
#define ON  1
#define NR_OF_GROUPINGS 10

#define INITIAL_CYCLE_TIME 300
#define NR_OF_CYCLES 5
#define MAX_CYCLE_TIME 600
#define CYCLE_FACTOR 2

volatile int cycleTime = INITIAL_CYCLE_TIME; // Initial value is used when no external clock is connected.
volatile uint32_t previousTime = 0;
volatile uint32_t thisTime = 0;
volatile bool ledState = false;
volatile unsigned long sumTime = 0L;
volatile unsigned long oldTime = millis();
volatile byte irqCounter = 0;

void clockISR() {
  // We measure the cycle time in MILLI seconds.
  thisTime = millis();
  // If there is an IRQ then increment the counter.
  irqCounter++;
  sumTime += (thisTime - oldTime);
  oldTime = thisTime;
  // We use the mean of the summed values of several cycle times
  // as the cycle time.
  if (irqCounter > NR_OF_CYCLES) {
    cycleTime = sumTime / irqCounter;
    if (cycleTime > MAX_CYCLE_TIME) {
      cycleTime = MAX_CYCLE_TIME;
    }
    irqCounter = 0;
    sumTime = 0;
  }
  debug_print("I");
  ledState = !ledState;
}

int computeBrightness() {
  // Calculate the mean brightness from 10 buffered values.
  // Scale the brightness value between MIN_BRIGHTNESS and MAX_BRIGHTNESS.
  const int MEAN_BRIGHTNESS_AT_DAYTIME = 650;
  static int brMin = 1024, brMax = 0;
  int scaledBr;
  static int summedBrightness = 9 * MEAN_BRIGHTNESS_AT_DAYTIME;
  int roomBrightness = analogRead(LIGHT_SENSOR);
  summedBrightness += roomBrightness;
  roomBrightness = summedBrightness / 10;
  summedBrightness -= roomBrightness;
  brMin = min(roomBrightness, brMin);
  brMax = max(roomBrightness, brMax);
  if (brMax != brMin) {
    scaledBr = map(roomBrightness, brMin, brMax, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  } else {
    scaledBr = 0;
  }
  //debug_print5("%4d > %4d -> %d  < %4d\n", brMin, roomBrightness, scaledBr, brMax);
  return(scaledBr);
}

void tickBrightness() {
  // Adjust led brightness every 2 seconds.
  static long t = 0L;
  if ((millis() - t) > 2000) {
    int brightness = computeBrightness();
    //debug_print2("brightness: %3d\n", brightness);
    display->setAllBrightnessValues(brightness);
    display->tick();
    t = millis();
  }
}

void tickBuiltInLed() {
  // Toggle led to indicate that device is alive.
  static long t = 0L;
  if ((millis() - t) > 1000) {
    t = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}

#ifdef TESTING
  void setup() {
    debug_begin(230400);
    debug_print("Begin of test setup.\n");
    pinMode(LIGHT_SENSOR, INPUT);        // LDR resistor divider
    pinMode(EXT_CLOCK_IN, INPUT_PULLDOWN); // External clock in
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Create display and WOPR.
    display = new Display(MAX7219_DIN_0_PIN, MAX7219_CLOCK_PIN, MAX7219_LOAD_PIN, NR_OF_DEVICES);
    printLedIndices();
    wopr = new Wopr(true);
    wopr->showVersion(VERSION_LSB, VERSION_MSB);
    wopr->test();
    debug_print("End of test setup.\n");
  }
#else
  void setup() {
    debug_begin(230400);
    debug_print("Begin of setup.\n");

    debug_print("Defining input and output ports.\n");
    pinMode(LIGHT_SENSOR, INPUT);        // LDR resistor divider
    pinMode(EXT_CLOCK_IN, INPUT_PULLDOWN); // External clock in
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Create display and WOPR.
    display = new Display(MAX7219_DIN_0_PIN, MAX7219_CLOCK_PIN, MAX7219_LOAD_PIN, NR_OF_DEVICES);
    printLedIndices();
    wopr = new Wopr();
    wopr->test();
    wopr->showVersion(VERSION_LSB, VERSION_MSB);
    delete wopr;
    wopr = new Wopr();
    debug_print("Attaching interrupt\n");
    attachInterrupt(digitalPinToInterrupt(EXT_CLOCK_IN), clockISR, RISING);

    #ifdef MAKE_VIDEO
      display->setAllBrightnessValues(0);
    #else
      tickBrightness();
    #endif
    debug_print("End of setup.\n");
  }
#endif

#ifdef TESTING
  void loop() {
    // Whenever 1 second has passed, we run the test
    static unsigned long refreshTimer = 0L;
    if ((millis() - refreshTimer) > 1000L) {
      refreshTimer = millis();
      wopr->test();
      tickBuiltInLed();
    }
  }
#else
  void loop() {
    static unsigned long previousTimeMain = 0L;
    static unsigned long oldTickTime = millis();
    unsigned long thisTimeMain = millis();
    unsigned long timeDiffMain = thisTimeMain - previousTimeMain;
    bool anyChanges = false;
    static bool tenTimes = true;
    // Bool used at startup to force all 10 groups to be updated once in 10 * 10 seconds.
    // After that the group to be updated will be chosen using a random number.
    static byte groupNr = -1;

    // Whenever 50 clock cycles have passed, we replace a random group's pattern and algoritm.
    // However, we force all 10 groups to be changed once from boot.
    #ifdef GROUP_REFRESH
      static unsigned long refreshTimer = millis();
      if ((millis() - refreshTimer) > 50L * uint32_t(cycleTime)) {
        debug_print("R");
        refreshTimer = millis();
        if (tenTimes) {
          groupNr++;
          if (groupNr > NR_OF_GROUPINGS - 1) {
            groupNr = 0;
            tenTimes = false;
          }
        } else {
          groupNr = randomNumberGenerator.getRandomNumber(0, NR_OF_GROUPINGS, 4);
        }
        debug_print2("Changing group %d\n", groupNr);
        wopr->replaceGroup(groupNr);
      }
    #endif

    if (timeDiffMain > 1000L) { // Adapt brightness every second.
      #ifndef MAKE_VIDEO // Keep led brightness as low as possible.
        #ifdef DYNAMIC_BRIGHTNESS
          tickBrightness();
        #endif
      #endif
      previousTimeMain = thisTimeMain;
    }

    // Show we are alive by flashing the LED on the BluePill
    tickBuiltInLed();

    if ( int(millis() - oldTickTime) > (cycleTime / CYCLE_FACTOR) ) {
      anyChanges = wopr->tick();
      debug_print(".");
      if (anyChanges) {
        wopr->show();
      }
      oldTickTime = millis();
    }
  }
#endif