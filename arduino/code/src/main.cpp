// vim:sts=4:sw=4

// WOPR-O-Matic
// Author: Jos Bouten
// Date: March 2024
// Version: v0.1

// WOPR-O-Matic is a display consisting of 104 red and 88 yellow LEDs controlled by three MAX7219
// LED controllers using the LedControl library (by Eberhard Fahle) runnng on an arduino nano.
// The arrangement of the LEDs is based on the WOPR computer from the movie 'WAR Games'.

// The LEDs are controlled by random numbers, but care was taken to make the LEDs change for
// short periods of fast changes and longer periods of slow changes.

// The pace of the pattern changes can be controlled by supplying a clock pulse to the 
// external input. The number of beats this clock makes per minute (BPM) drives the pattern speed
// but the BPM is clamped to a maximum of MAX_BPM. The BPM is MEAN_BPM if no clock
// pulse is supplied.

#define DEBUG

#include <Arduino.h>
#include "LedControl.h"
#include "debug.hpp"

// #define HW_SPI
// Set this when using version 0.2s PCB

#ifdef HW_SPI
  #define MAX7219_DIN_0_PIN 11 // MOSI
  #define MAX7219_LOAD_PIN   3 // D3
  #define MAX7219_CLOCK_PIN 13 // CLK
#else
  #define MAX7219_DIN_0_PIN 5 // D5
  #define MAX7219_LOAD_PIN  3 // D3
  #define MAX7219_CLOCK_PIN 4 // CLK
#endif

#define NR_OF_DEVICES  3 // Three MAX7219 ICs are used to control the 3x64=192 LEDs

#define EXT_CLOCK_IN   2 // Extern clock is connected to INT0, so that it can trigger an interrupt.
#define LIGHT_SENSOR  A2 // LDR + resistor divider

#define CNT1 20
#define CNT2 60
#define MIN_TIME 5
#define MAX_TIME 100

#define MIN_BR 0 // max brightness of LEDs
#define MAX_BR 4 // max brightness of LEDs

#define INITIAL_CYCLE_TIME 600
#define NR_OF_CYCLES 5
#define MAX_CYCLE_TIME 600
#define CYCLE_FACTOR 6

// Do not change anything below this line
// --------------------------------------

LedControl *ledControl;

volatile int cycleTime = INITIAL_CYCLE_TIME; // Initial value is used when no external clock is connected.
volatile uint32_t previousTime = 0;
volatile uint32_t thisTime = 0;
volatile bool ledState = false;
volatile unsigned long sumTime = 0L;
volatile unsigned long oldTime = millis();
volatile byte irqCounter = 0;
volatile bool running = false;
volatile uint32_t clockPulseCnt = 0;

void(* resetFunc) (void) = 0; // Declare reset function at address 0, used to reset the arduino.

void clockISR() {
  // This ISR takes care of clock pulses either from the tap button or the clock input.
  // Its goal is to update the cycle time which is used to compute the delay time between pattern steps.

  // We measure the cycle time in MILLI seconds.
  thisTime = millis();
  irqCounter++;
  sumTime += (thisTime - oldTime);
  oldTime = thisTime;
  // We use the mean of the summed values of several cycle times 
  // as the time between pattern steps.
  if (irqCounter > NR_OF_CYCLES) {
    cycleTime = sumTime / irqCounter;
    irqCounter = 0;
    sumTime = 0;
  }
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);
}

bool randomBool() {
  if (random(1, 11) > 7) {
    debug_print("slow\n");
    return(true);
  } else {
    debug_print("fast\n");
    return(false);
  }
}

int rndDelay(int min, int max) {
  static int slowCnt = 0;
  static bool slowBool = randomBool();
  static int cnt1 = CNT1 + random(1, 20);
  static int cnt2 = CNT2 + random(1, 40);
  if (slowBool)  {
    slowCnt++;
    // We want a long series (because they take less time) of fast changes.
    if (slowCnt > cnt2) {
      slowBool = randomBool();
      slowCnt = 0;
    } // we go slow
    return(random(max / 2, 2 * max));
  } else {
    slowCnt++;
    // We want a short series (because they take more time) of slow changes.
    if (slowCnt > cnt1) {
      slowBool = randomBool();
      slowCnt = 0;
    } // we go fast.
    return(random(min, max / 2));
  }
}

void setBrightness(int brightness) {
  for (int deviceNr = 0; deviceNr < NR_OF_DEVICES; deviceNr++) {
    ledControl->setIntensity(deviceNr, brightness);
  }
}

int computeBrightness() {
  // Calculate the mean brightness from 10 buffered values.
  // Scale the brightness value between MIN_BR and MAX_BR.
  static int brMin = 1024, brMax = 0;
  int scaledBr;
  static int summedBrightness = 9 * 660;
  int roomBrightness = analogRead(LIGHT_SENSOR);
  summedBrightness += roomBrightness;
  roomBrightness = summedBrightness / 10;
  summedBrightness -= roomBrightness;
  brMin = min(roomBrightness, brMin);
  brMax = max(roomBrightness, brMax);
  if (brMax != brMin) {
    scaledBr = map(roomBrightness, brMin, brMax, MIN_BR, MAX_BR);
  } else {
    scaledBr = 0;
  }
  //debug_print5("%4d < %4d -> %d  < %4d\t", brMin, roomBrightness, scaledBr, brMax);
  return(scaledBr);
}

void flashLed() {
  digitalWrite(LED_BUILTIN, HIGH);
  debug_print("Flashing builtin led 2 times");
  delay(100);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(100);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(100);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}


void loop() {
  static long oldTime = 0L;
  static bool oldLedState = false;
  static byte cntr = 0;
  int delayTime = 0;

  // Adjust led brightness every 2 seconds.
  if ((millis() - oldTime) > 2000) {
    setBrightness(computeBrightness());
    oldTime = millis();
  }

  if (random(1, 10) > 2) {
    ledControl->setRow(cntr, random(0, 8), random(0, 0xFF));
    delayTime = rndDelay(cycleTime / 4, cycleTime / 2);
  }
  else {
    ledControl->setColumn(cntr, random(0, 8), random(0, 0xFF));
    delayTime = rndDelay(cycleTime / 4, cycleTime / 2);
  }
  cntr = (byte) random(0, 3);


  //debug_print2("delayTime: %d\n", delayTime);
  delay(delayTime);

  if (oldLedState != ledState) {
    digitalWrite(LED_BUILTIN, ledState);
    oldLedState = ledState;
  }
}

void setup() {
  debug_begin(230400);
  debug_print("Begin of setup\n");
  ledControl = new LedControl(MAX7219_DIN_0_PIN, MAX7219_CLOCK_PIN, MAX7219_LOAD_PIN, NR_OF_DEVICES);

  for (byte controller = 0; controller < NR_OF_DEVICES; controller++) {
    ledControl->shutdown(controller, false);
    // Set the brightness to a medium values
    ledControl->setIntensity(controller, 2);
    // and clear the display.
    ledControl->clearDisplay(controller); 
  } 
  #ifdef DEBUG
    print_freeram();
  #endif

  pinMode(LED_BUILTIN, OUTPUT);
  flashLed();
  
  debug_print("Defining inputs\n");
  pinMode(LIGHT_SENSOR, INPUT);        // LDR resistor divider
  pinMode(EXT_CLOCK_IN, INPUT_PULLUP); // External clock in
  debug_print("Attaching interrupt\n");
  attachInterrupt(digitalPinToInterrupt(EXT_CLOCK_IN), clockISR, FALLING);

  // We seed the random number generator so that if we have more than one
  // they do not show the same patterns.
  // Input A3 is not connected to anything, so when reading it we should get 
  // some random number we can use as a seed.
  randomSeed(analogRead(A3));

  debug_print("Clearing all LEDs\n");  
  debug_print("End of setup\n");
}