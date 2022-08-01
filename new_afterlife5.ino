/****************************************************************************************
   Toby Brandon - Arduino Source for 2022 Ghostbusters Proton Pack
   LED pinout 2 / 3 / 4 using addressable LED's (NewPixels) and 1 Adafruit Jewel for the wand gun
   Pinout 2 - Cyclotron LEDs 40 ring (change to suit)
   Pintout 3 - powercell - 14 leds
    Pintout 4 Cyclotron Vent 0 - 11, 12 Slow-Blow, 13, 14, 15 Wand Vent, 16 White LED, 17 White LED2, 18, Yellow LED, , 19 Orange Hat LED, 20-26 NeoPixel Jewel
   Sound Module -  serial mp3 player - caltex yx5300
   Some Reference Code from Eric Banker (Wand LED write helper function)
   Some Reference Code from mikes11 (modified led write helpers)
   Some Reference Code from cfunseth (afterlife cyclotron library)

   modifications and additions added from various sources and help from friends and the ghostbuster community

   thank you!

*/


#include "afterlife_cyclotron.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h> // Include the I2C library (required)
#include <SparkFunSX1509.h> // Include SX1509 library



#define PIN 2           // Which pin on the Arduino is connected to the NeoPixels?
#define HEAT_PIN  9     // Which pin on the Arduino is connected to the "overheat" signal?
#define NUMPIXELS 40    // How many NeoPixels are attached to the Arduino?
#define BRIGHTNESS 150  // How bright should the pixels be? (min = 0, max = 255)
#define GROUP 1         // How big of a group of pixels do you want to rotate?
#define INIT_SPD  190   // How slow do you want the animation to rotate at the beginning of the boot? (higher = slower, 255 max)
#define IDLE_SPD  5   // How fast do you want the animation to rotate during "normal" operation (lower = faster, 0 min)
#define HEAT_SPD 0      // How fast do you want the animation to rotate at overheat? (lower = faster, 0 min)
#define BOOT_DLY  17000  // How long do you want the boot animation to last?
#define HEAT_DLY  15000  // How long should the "overheat" ramp up last?
#define BRIGHTNESS_LOW 0
#define shutoff_DLY 6000

// Create a new cyclotron object
Cyclotron cyclotron(PIN, NUMPIXELS, GROUP, INIT_SPD);

// Create a new POWERCELL object
#define PM_PIN      3        // input pin Powercell strips are attached to
#define PM_PIXELS   14     // total number of neopixels in powercell

Adafruit_NeoPixel powercell = Adafruit_NeoPixel(PM_PIXELS, PM_PIN, NEO_GRB + NEO_KHZ800);
int  PM_TIME =  800;       // time to max power in milliseconds
int meterlevel = 0;
unsigned long lastMeterResetTime;

int seq_1_current = 0;  // current led in sequence 1
unsigned long firing_interval = 40;     // interval at which to cycle firing lights on the bargraph. We update this in the loop to speed up the animation so must be declared here (milliseconds).

const int ventStart = 0;
const int ventEnd = 11;
const int SloBloLED = 12;
const int WandVentstart = 13;
const int WandVentsend = 15;
const int ThemeLED = 16;
const int WhiteLED1 = 17;
const int WhiteLED2 = 18;
const int OrangeHatLED = 19;
const int GunLEDStart = 20;
const int GunLEDEnd = 26;

// Vent + Wand LED Count
const int NeoPixelLEDCount2 = 27;

#define NEO_WAND 4 // for powercell
Adafruit_NeoPixel wandLights = Adafruit_NeoPixel(NeoPixelLEDCount2, NEO_WAND, NEO_GRB + NEO_KHZ800);


// Startup / Shutdown functions that clear all the LEDs/neopixels
void clearLEDs()
{
  for (int i = 0; i <= NeoPixelLEDCount2; i++) {
    wandLights.setPixelColor(i, 0);
  }
}
void clearGunLEDs()
{
  for (int i = GunLEDStart; i <= GunLEDEnd; i++) {
    wandLights.setPixelColor(i, 0);
  }
}
////   how to write -         setWandLightState(12, 2, 0);    //  set slo blo orange  (1 - pixel, 2 - state, 3 - millis/time

/*************** Wand Light Helpers *********************
    Modified from Eric Bankers source
*/
unsigned long prevFlashMillis = 0; // Last time we changed wand sequence 1
unsigned long prevFlashMillis2 = 0; // Last time we changed wand sequence 2
unsigned long prevFlashMillis3 = 0; // Last time we changed wand sequence 3
unsigned long prevFlashMillis4 = 0; // Last time we changed wand sequence 4
unsigned long prevFlashMillis5 = 0; // Last time we changed wand sequence 5
unsigned long prevFlashMillis6 = 0; // Last time we changed wand sequence 6
unsigned long prevFlashMillis7 = 0; // Last time we changed wand sequence 7
unsigned long prevFlashMillis8 = 0; // Last time we changed wand sequence 8
unsigned long prevFlashMillis9 = 0; // Last time we changed wand sequence 8

bool flashState1 = false;
bool flashState2 = false;
bool flashState3 = false;
bool flashState4 = false;
bool flashState5 = false;
bool flashState6 = false;
bool flashState7 = false;
bool flashState8 = false;
bool flashState9 = false;
const unsigned long wandFastFlashInterval = 100; // interval at which we flash the top led on the wand
const unsigned long wandMediumFlashInterval = 400; // interval at which we flash the top led on the wand
const unsigned long wandSlowFlashInterval = 800; // interval at which we flash the slow led on the wand
void setWandLightState(int lednum, int state, unsigned long currentMillis) {
  switch ( state ) {
    case 0: // set led red
      wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
      break;
    case 1: // set led white
      wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
      break;
    case 2: // set led orange
      wandLights.setPixelColor(lednum, wandLights.Color(255, 127, 0));
      break;
    case 3: // set led blue
      wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 255));
      break;
    case 4: // set led off
      wandLights.setPixelColor(lednum, 0);
      break;
    case 5: // fast white flashing
      if ((unsigned long)(currentMillis - prevFlashMillis) >= wandFastFlashInterval) {
        prevFlashMillis = currentMillis;
        if ( flashState1 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
          flashState1 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState1 = false;
        }
      }
      break;
    case 6: // slower orange flashing
      if ((unsigned long)(currentMillis - prevFlashMillis2) >= wandMediumFlashInterval) {
        prevFlashMillis2 = currentMillis;
        if ( flashState2 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 051, 0));
          flashState2 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState2 = false;
        }
      }
      break;
    case 7: // medium red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis3) >= wandMediumFlashInterval) {
        prevFlashMillis3 = currentMillis;
        if ( flashState3 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState3 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState3 = false;
        }
      }
      break;
    case 8: // fast red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis4) >= wandFastFlashInterval) {
        prevFlashMillis4 = currentMillis;
        if ( flashState4 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState4 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState4 = false;
        }
      }
      break;
    case 9: // set LED green
      wandLights.setPixelColor(lednum, wandLights.Color(0, 255, 0));
      break;
    case 10: // slower orange flashing
      if ((unsigned long)(currentMillis - prevFlashMillis5) >= wandMediumFlashInterval) {
        prevFlashMillis5 = currentMillis;
        if ( flashState5 == false ) {
          wandLights.setPixelColor(lednum, 0);
          flashState5 = true;
        } else {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
          flashState5 = false;
        }
      }
      break;
    case 11: // slower orange flashing from red
      if ((unsigned long)(currentMillis - prevFlashMillis6) >= wandMediumFlashInterval) {
        prevFlashMillis6 = currentMillis;
        if ( flashState6 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(0, 255, 0));
          flashState6 = true;
        } else {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState6 = false;
        }
      }
      break;
    case 12: // slowest red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis7) >= wandSlowFlashInterval) {
        prevFlashMillis7 = currentMillis;
        if ( flashState7 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState7 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState7 = false;
        }
      }
      break;
    case 13: // slower white flashing
      if ((unsigned long)(currentMillis - prevFlashMillis7) >= wandSlowFlashInterval) {
        prevFlashMillis8 = currentMillis;
        if ( flashState8 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
          flashState8 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState8 = false;
        }
      }
      break;
    case 14: // medium white flashing
      if ((unsigned long)(currentMillis - prevFlashMillis7) >= wandMediumFlashInterval) {
        prevFlashMillis9 = currentMillis;
        if ( flashState9 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
          flashState9 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState9 = false;
        }
      }
      break;

  }
  wandLights.show();
}

/***************** Vent Light *************************
  Modified from Eric Banker's source code
*/
void setVentLightState(int startLed, int endLed, int state ) {
  switch ( state ) {
    case 0: // set all leds to white
      for (int i = ventStart; i <= ventEnd; i++) {
        wandLights.setPixelColor(i, wandLights.Color(255, 255, 255));
      }
      // Set the relay to on while venting. If relay is off set the pin LOW

      break;
    case 1: // set all leds to blue
      for (int i = ventStart; i <= ventEnd; i++) {
        wandLights.setPixelColor(i, wandLights.Color(0, 0, 255));
      }
      // Set the relay to on while venting. If relay is off set the pin LOW

      break;
    case 2: // set all leds off
      for (int i = ventStart; i <= ventEnd; i++) {
        wandLights.setPixelColor(i, 0);
      }
      // Set the relay to OFF while not venting. If relay is onf set the pin HIGH

      break;
  }
  wandLights.show();
}

/*************** Firing Animations *********************/
unsigned long prevFireMillis = 0;
const unsigned long fire_interval = 50;     // interval at which to cycle lights (milliseconds).
int fireSeqNum = 0;
int fireSeqTotal = 5;

void fireStrobe(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevFireMillis) >= fire_interval) {
    prevFireMillis = currentMillis;

    switch ( fireSeqNum ) {
      case 0:
        wandLights.setPixelColor(20, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(21, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(22, 0);
        wandLights.setPixelColor(23, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(24, 0);
        wandLights.setPixelColor(25, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(26, 0);
        break;
      case 1:
        wandLights.setPixelColor(20, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(21, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(22, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(23, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(24, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(25, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(26, wandLights.Color(255, 255, 255));
        break;
      case 2:
        wandLights.setPixelColor(20, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(21, 0);
        wandLights.setPixelColor(22, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(23, 0);
        wandLights.setPixelColor(24, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(25, 0);
        wandLights.setPixelColor(26, wandLights.Color(255, 0, 0));
        break;
      case 3:
        wandLights.setPixelColor(20, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(21, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(22, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(23, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(24, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(25, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(26, wandLights.Color(255, 255, 255));
        break;
      case 4:
        wandLights.setPixelColor(20, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(21, 0);
        wandLights.setPixelColor(22, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(23, 0);
        wandLights.setPixelColor(24, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(25, 0);
        wandLights.setPixelColor(26, wandLights.Color(255, 255, 255));
        break;
      case 5:
        wandLights.setPixelColor(20, wandLights.Color(255, 0, 255));
        wandLights.setPixelColor(21, wandLights.Color(0, 255, 0));
        wandLights.setPixelColor(22, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(23, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(24, wandLights.Color(255, 0, 255));
        wandLights.setPixelColor(25, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(26, wandLights.Color(0, 0, 255));
        break;
    }
    wandLights.show();

    fireSeqNum++;
    if ( fireSeqNum > fireSeqTotal ) {
      fireSeqNum = 0;
    }
  }
}

#include <MD_YX5300.h>
#include <SoftwareSerial.h>

// Connections for serial interface to the YX5300 module
const uint8_t ARDUINO_RX = 19;    // connect to TX of MP3 Player module
const uint8_t ARDUINO_TX = 18;    // connect to RX of MP3 Player module

SoftwareSerial  MP3Stream(ARDUINO_RX, ARDUINO_TX);  // MP3 player serial stream for comms

const uint8_t PLAY_FOLDER = 1;   // tracks are all placed in this folder


// Define global variables
MD_YX5300 mp3(MP3Stream);

bool playerPause = true;  // true if player is currently paused
int readVol = 30;
int song = 0;
int playing = 1;

/********sd card sounds in order - folder (01)********
  wand on - 1
  venting - 2
  higher and higher song - 3
  epic gb theme - 4
  on our own song - 5
  under the floor song - 6
  punkrock gb - 7
  startup - 8
  shutdown - 9
  idle - 10
  safe 1 - 11
  safe 2 - 12
  error - 13
  endfire start - 14
  endfire silent - 15
  click - 16
***************mp3 commands*********************
  mp3.playStart();
  mp3.playPause();
  mp3.playStop();
  mp3.playNext();
  mp3.playPrev();
  mp3.playTrack(t);
  mp3.playSpecific(fldr, file);
  mp3.playFolderRepeat(fldr);
  mp3.playFolderShuffle(fldr);
  mp3.playTrackRepeat(file);
  mp3.volumeMute(cmd != 0);
  mp3.volume(v);
   mp3.volumeInc();
  mp3.volumeDec();
  mp3.queryEqualizer();
  mp3.queryFile();
  mp3.queryStatus();
  mp3.queryVolume();
  mp3.queryFolderCount();
  mp3.queryFilesCount();
  mp3.queryFolderFiles(fldr);
  mp3.sleep();
  mp3.wakeUp();
  mp3.reset();
  mp3.equalizer(e);
  mp3.shuffle(cmd != 0);
  mp3.repeat(cmd != 0);
  setSynchMode(cmd != 0);
  setCallbackMode(cmd != 0);
****************/



void lightsout() {
  unsigned long currentMillis = millis();
  for (int i = 0; i < PM_PIXELS; i++)  {
    powercell.setPixelColor(i,  powercell.Color(0, 0, 0));

  
    powercell.show();

  }
}

void doCycle() {
  lastMeterResetTime = millis();
  meterlevel = 0;
}


void power_cell_idle()
{
  unsigned long currentMillis = millis();
  meterlevel = ((currentMillis - lastMeterResetTime) * PM_PIXELS) / PM_TIME;
  if (meterlevel > PM_PIXELS)
    doCycle();
  for (int i = 0; i < PM_PIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    if (meterlevel >= i + 1)
      powercell.setPixelColor(i, powercell.Color(0, 0, 200));
    else
      powercell.setPixelColor(i, powercell.Color(0, 0, 0));
  }
  powercell.show();
}



//********** PACK STATES / BUTTON STATE ***********************
bool songplaying = true;
bool poweroff = true;
bool PowerBooted = false;



// ******************* physical switch states ******************* //

bool wand_on  = false;
bool startpack = false;
bool sw20 = true;
bool sw21 = true;
bool sw30 = true;
bool sw31 = true;
bool sw40 = true;
bool sw41 = true;

/********** timing states  ***************************************/

// timer trigger times/states
unsigned long firingStateMillis;
const unsigned long firingWarmWaitTime = 5000;  // how long to hold down fire for lights to speed up
const unsigned long firingWarnWaitTime = 10000;  // how long to hold down fire before warning sounds



/****************************************************************/

const int STARTPACK_SWITCH = 5;
const int STARTWAND_SWITCH = 6;
const int SAFETY_ONE = 7;
const int SAFETY_TWO = 8;
// HEAT_PIN = 9;  // FIRE
const int MUSIC_SWITCH = 10; // FIRE2

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte ADDRESS = 0x3E;  // SX1509 I2C address
SX1509 io; // Create an SX1509 object to be used throughout

// bargraph helper variables
const int num_led = 12; // total number of leds in bar graph

// SX1509 pin definitions for the leds on the graph:
// SX1509 pin definitions for the leds on the graph:
const byte BAR_01 = 0;
const byte BAR_02 = 1;
const byte BAR_03 = 2;
const byte BAR_04 = 3;
const byte BAR_05 = 4;
const byte BAR_06 = 5;
const byte BAR_07 = 6;
const byte BAR_08 = 7;
const byte BAR_09 = 8;
const byte BAR_10 = 9;
const byte BAR_11 = 10;
const byte BAR_12 = 11;


/*************** Bar Graph Animations *********************/
// This is the idle sequence
unsigned long prevBarMillis_on = 0;          // bargraph on tracker
const unsigned long pwrcl_interval = 25;     // interval at which to cycle lights (milliseconds).
bool reverseSequenceOne = false;

void barGraphSequenceOne(unsigned long currentMillis) {
  // normal sync animation on the bar graph
  if ((unsigned long)(currentMillis - prevBarMillis_on) > pwrcl_interval) {
    // save the last time you blinked the LED
    prevBarMillis_on = currentMillis;

    if ( reverseSequenceOne == false ) {
      switch_graph_led(seq_1_current, HIGH);
      seq_1_current++;
      if ( seq_1_current > num_led ) {
        reverseSequenceOne = true;
      }
    } else {
      switch_graph_led(seq_1_current, LOW);
      seq_1_current--;
      if ( seq_1_current < 0  ) {
        reverseSequenceOne = false;
      }
    }
  }
}

// This is the firing sequence
unsigned long prevBarMillis_fire = 0; // bargraph firing tracker
int fireSequenceNum = 1;

void barGraphSequenceTwo(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevBarMillis_fire) > firing_interval) {
    // save the last time you blinked the LED
    prevBarMillis_fire = currentMillis;

    switch (fireSequenceNum) {
      case 1:
        switch_graph_led(1, HIGH);
        switch_graph_led(12, HIGH);
        switch_graph_led(11, LOW);
        switch_graph_led(2, LOW);


        fireSequenceNum++;
        break;
      case 2:
        switch_graph_led(2, HIGH);
        switch_graph_led(11, HIGH);
        switch_graph_led(1, LOW);
        switch_graph_led(12, LOW);

        fireSequenceNum++;
        break;
      case 3:
        switch_graph_led(3, HIGH);
        switch_graph_led(10, HIGH);
        switch_graph_led(2, LOW);
        switch_graph_led(11, LOW);


        fireSequenceNum++;
        break;
      case 4:
        switch_graph_led(4, HIGH);
        switch_graph_led(9, HIGH);
        switch_graph_led(3, LOW);
        switch_graph_led(10, LOW);


        fireSequenceNum++;
        break;
      case 5:
        switch_graph_led(5, HIGH);
        switch_graph_led(8, HIGH);
        switch_graph_led(4, LOW);
        switch_graph_led(9, LOW);


        fireSequenceNum++;
        break;
      case 6:
        switch_graph_led(6, HIGH);
        switch_graph_led(7, HIGH);
        switch_graph_led(5, LOW);
        switch_graph_led(8, LOW);

        fireSequenceNum++;
        break;
      case 7:
        switch_graph_led(5, HIGH);
        switch_graph_led(8, HIGH);
        switch_graph_led(6, LOW);
        switch_graph_led(7, LOW);


        fireSequenceNum++;
        break;
      case 8:
        switch_graph_led(4, HIGH);
        switch_graph_led(9, HIGH);
        switch_graph_led(5, LOW);
        switch_graph_led(8, LOW);

        fireSequenceNum++;
        break;
      case 9:
        switch_graph_led(3, HIGH);
        switch_graph_led(10, HIGH);
        switch_graph_led(4, LOW);
        switch_graph_led(9, LOW);

        fireSequenceNum++;
        break;
      case 10:
        switch_graph_led(2, HIGH);
        switch_graph_led(11, HIGH);
        switch_graph_led(3, LOW);
        switch_graph_led(10, LOW);

        fireSequenceNum++;
        break;
      case 11:
        switch_graph_led(1, HIGH);
        switch_graph_led(12, HIGH);
        switch_graph_led(2, LOW);
        switch_graph_led(11, LOW);


        fireSequenceNum = 1;
        break;
    }
  }
}

/************************* Shutdown and helper functions ****************************/
void shutdown_leds() {
  // reset the sequence
  seq_1_current = 1;
  fireSequenceNum = 1;

  // shut all led's off
  for (int i = 1; i <= 12; i++) {
    switch_graph_led(i, LOW);
  }
}
void switch_graph_led(int num, int state) {
  switch (num) {
    case 1:
      io.digitalWrite(BAR_01, state);
      break;
    case 2:
      io.digitalWrite(BAR_02, state);
      break;
    case 3:
      io.digitalWrite(BAR_03, state);
      break;
    case 4:
      io.digitalWrite(BAR_04, state);
      break;
    case 5:
      io.digitalWrite(BAR_05, state);
      break;
    case 6:
      io.digitalWrite(BAR_06, state);
      break;
    case 7:
      io.digitalWrite(BAR_07, state);
      break;
    case 8:
      io.digitalWrite(BAR_08, state);
      break;
    case 9:
      io.digitalWrite(BAR_09, state);
      break;
    case 10:
      io.digitalWrite(BAR_10, state);
      break;
    case 11:
      io.digitalWrite(BAR_11, state);
      break;
    case 12:
      io.digitalWrite(BAR_12, state);
      break;
  }
}

/************************* WAND LED PART functions ****************************/

/* Wand LED assighment


  const int ventStart = 0;
  const int ventEnd = 11;
  const int SloBloLED = 12;
  const int WandVentstart = 13;
  const int WandVentsend = 15;
  const int ThemeLED = 16;
  const int WhiteLED1 = 17;
  const int WhiteLED2 = 18;
  const int OrangeHatLED = 19;
  const int GunLEDStart = 20;
  const int GunLEDEnd = 27;

*/

void WANDcharging() {
  unsigned long currentMillis = millis();
  setWandLightState(12, 12, currentMillis);    // sloblo red slow flash
  setWandLightState(13, 4, 0);    //  vent start off
  setWandLightState(14, 4, 0);    //  vent mid   off
  setWandLightState(15, 4, 0);    // vent end   off
  setWandLightState(16, 4, 0 );     // theme led off
  setWandLightState(17, 4, 0);    //  white led off
  setWandLightState(18, 4, 0);    // white led off
  setWandLightState(19, 13, currentMillis); // Set orange hat barrel flashing
  setVentLightState(ventStart, ventEnd, 2);


}
void WANDledstate2() {
  unsigned long currentMillis = millis();
  setWandLightState(12, 0, 0);    // sloblo red
  setWandLightState(13, 4, 0);    //  vent start white off
  setWandLightState(14, 4, 0);    //  vent mid   white off
  setWandLightState(15, 4, 0);    // vent end  white off
  setWandLightState(16, 10, currentMillis );     // theme led slow flash orange
  setWandLightState(17, 4, currentMillis);    //  white led flash
  setWandLightState(18, 14, currentMillis);    // white led flash
  setWandLightState(19, 6, currentMillis); // Set orange hat barrel flashing
  setVentLightState(ventStart, ventEnd, 2);

}

void WANDledstate3() {
  unsigned long currentMillis = millis();
  setWandLightState(12, 0, 0);    // sloblo red
  setWandLightState(13, 1, 0);    //  vent start white
  setWandLightState(14, 1, 0);    //  vent mid   white
  setWandLightState(15, 1, 0);    // vent end   white
  setWandLightState(16, 10, currentMillis );     // theme led flash orange
  setWandLightState(17, 14, currentMillis);    //  white led flash slow
  setWandLightState(18, 5, currentMillis);    // white led flash fals
  setWandLightState(19, 6, currentMillis); // Set orange hat barrel flashing
  setVentLightState(ventStart, ventEnd, 2);
}

void WANDLEDstateOFF() {

  setWandLightState(12, 4, 0);    //  set sloblo light off
  setWandLightState(13, 4 , 0);    //  set wand vent led off
  setWandLightState(14, 4, 0);   //  set wand vent led off
  setWandLightState(15, 4, 0);   //  set wand vent led off
  setWandLightState(16, 4, 0);    //  set sloblo  led off
  setWandLightState(17, 4, 0);     // Top LED off
  setWandLightState(18, 4, 0); // set back led off
  setWandLightState(19, 4, 0);    //   orange hat light off
  setVentLightState(ventStart, ventEnd, 2);  // off

}


void safety1_read() {
  if (!digitalRead(SAFETY_ONE)) {
    mp3.playTrack(11);

    WANDledstate2();
    power_cell_idle();
    // safety2_read ();
  }
  else

    power_cell_idle();
  mp3.playTrack(16);


}
void safety2_read() {
  if (!digitalRead(SAFETY_TWO)) {


    mp3.playTrack(12);
    power_cell_idle();
    WANDledstate3();
    fire_seq ();

  }
  else
    mp3.playTrack(16);
  power_cell_idle();

}
void fire_seq() {
  unsigned long currentMillis = millis();
  if (!digitalRead(HEAT_PIN)) {

        power_cell_idle();

    barGraphSequenceTwo(currentMillis);
    fireStrobe(currentMillis);
    mp3.playTrack(17);

  }
  else
    clearGunLEDs();
  mp3.playTrack(14);
  power_cell_idle();
  WANDledstate3();

}



// This is the bargraph CHARGING sequence
unsigned long prevBarCHARGEMillis = 0;          // bargraph on tracker
const unsigned long CHARGE_interval = 700;     // interval at which to charge lights (milliseconds).
bool CHARGESequenceOne = false;

void barGraphCHARGING(unsigned long currentMillis) {
  // normal sync animation on the bar graph

  if ((unsigned long)(currentMillis - prevBarCHARGEMillis) > CHARGE_interval) {
    // save the last time you blinked the LED
    prevBarCHARGEMillis = currentMillis;

    if ( CHARGESequenceOne == false )   {
      switch_graph_led(seq_1_current, HIGH);
      seq_1_current++;
      if ( seq_1_current > num_led ) {
        CHARGESequenceOne = true;
    }
    } else {
      switch_graph_led(seq_1_current, LOW);
      seq_1_current--;
      if ( seq_1_current < 0  ) {
        CHARGESequenceOne = false;
      }
    }
  }
}

void playAudio(int playing) {
  // stop track if one is going
  if (playing == 1) {
   mp3.playPause();
  }

  // now go play
  mp3.playStart();
}

void setup() {
  //Setup and start the cyclotron
  //cyclotron.setBrightness(0, 0);
  cyclotron.start();
 

  //start the powercell
  powercell.begin();
  lastMeterResetTime = millis();
  powercell.clear();

  // ***** Configure LED's in wand lights (Including Vent LEDs) ***** //
  wandLights.begin();
  wandLights.setBrightness(240);
  wandLights.show();

  // initialize global libraries
  MP3Stream.begin(MD_YX5300::SERIAL_BPS);
  mp3.begin();
  mp3.setSynchronous(true);

  //  for signal testing //
  pinMode(13, OUTPUT);

  // set the modes for the switches/buttons
  pinMode(STARTPACK_SWITCH, INPUT_PULLUP);
  digitalWrite(STARTPACK_SWITCH, HIGH);
  // ***** Assign Proton Pack Switches / Buttons ***** //
  pinMode(STARTWAND_SWITCH, INPUT_PULLUP);
  digitalWrite(STARTWAND_SWITCH, HIGH);
  pinMode(SAFETY_ONE, INPUT_PULLUP);
  digitalWrite(SAFETY_ONE, HIGH);
  pinMode(SAFETY_TWO, INPUT_PULLUP);
  digitalWrite(SAFETY_TWO, HIGH);
  pinMode(HEAT_PIN, INPUT_PULLUP);
  digitalWrite(HEAT_PIN, HIGH);
  pinMode(MUSIC_SWITCH, INPUT_PULLUP);
  digitalWrite(MUSIC_SWITCH, HIGH);



  // Call io.begin(<address>) to initialize the SX1509. If it
  // successfully communicates, it'll return 1.
  if (!io.begin(ADDRESS)) {
    while (1) ; // If we fail to communicate, loop forever for now but it would be nice to warn the user somehow
  }

  // configuration for the bargraph LED's
  io.pinMode(BAR_01, OUTPUT);
  io.pinMode(BAR_02, OUTPUT);
  io.pinMode(BAR_03, OUTPUT);
  io.pinMode(BAR_04, OUTPUT);
  io.pinMode(BAR_05, OUTPUT);
  io.pinMode(BAR_06, OUTPUT);
  io.pinMode(BAR_07, OUTPUT);
  io.pinMode(BAR_08, OUTPUT);
  io.pinMode(BAR_09, OUTPUT);
  io.pinMode(BAR_10, OUTPUT);
  io.pinMode(BAR_11, OUTPUT);
  io.pinMode(BAR_12, OUTPUT);

  shutdown_leds();
}

//  to read vol with pot   -      readVol = map(analogRead(A0), 0, 1024, 0, 30); - to begin loop then add
//                                    mp3.setVol(/*VOL = */readVol);


void loop() {


  /*************************************************************************
                                   Main Loop Function
   *************************************************************************/

  /********sd card sounds in order - folder (01)********
    wand on - 1
    venting - 2
    higher and higher song - 3
    epic gb theme - 4
    on our own song - 5
    under the floor song - 6
    punkrock gb - 7
    startup - 8
    shutdown - 9
    idle - 10
    safe 1 - 11
    safe 2 - 12
    error - 13
    endfire start - 14
    endfire silent - 15
    click - 16
    blast - 17


  ***************mp3 commands*********************
    mp3.playStart();
    mp3.playPause();
    mp3.playStop();
    mp3.playNext();
    mp3.playPrev();
    mp3.playTrack(t);
    mp3.playSpecific(fldr, file);
    mp3.playFolderRepeat(fldr);
    mp3.playFolderShuffle(fldr);
    mp3.playTrackRepeat(file);
    mp3.volumeMute(cmd != 0);
    mp3.volume(v);
    mp3.volumeInc();
    mp3.volumeDec();
    mp3.queryEqualizer();
    mp3.queryFile();
    mp3.queryStatus();
    mp3.queryVolume();
    mp3.queryFolderCount();
    mp3.queryFilesCount();
    mp3.queryFolderFiles(fldr);
    mp3.sleep();
    mp3.wakeUp();
    mp3.reset();
    mp3.equalizer(e);
    mp3.shuffle(cmd != 0);
    mp3.repeat(cmd != 0);
    setSynchMode(cmd != 0);
    setCallbackMode(cmd != 0);

  ****************/

  // get the current time
  unsigned long currentMillis = millis();

cyclotron.update();
  // song selection
  int theme_switch = digitalRead(MUSIC_SWITCH);

  if (theme_switch == LOW  ) {
    if (songplaying == true)
      song ++;
    if (song > 6)song = 1;
    switch (song) {
      case 1:
        mp3.playTrack(3);
        break;
      case 2 :
        mp3.playTrack(4);
        break;
      case 3:
        mp3.playTrack(5);
        break;
      case 4 :
        mp3.playTrack(6);
        break;
      case 5 :
        mp3.playTrack(7);
        break;
      case 6:
        mp3.playFolderShuffle(1);
        break;
    }
    songplaying = false;
  } else songplaying = true;


  /********** PACK STATES / BUTTON STATE ***********************
    bool poweroff = true;
    bool PowerBooted = false;

    bool songplaying = true;

    // ******************* physical switch states ******************* //
    bool wand_on  = false
    bool startpack = false;

  **/      // ******************* lets power up the pack! ******************* /
  if (!digitalRead(STARTPACK_SWITCH)) {
   if (poweroff);
      poweroff = false;
      
    WANDcharging();
   // mp3.playTrack(8);    }


  cyclotron.setSpeed(IDLE_SPD, BOOT_DLY);
  cyclotron.setBrightness(BRIGHTNESS, BOOT_DLY);
    barGraphCHARGING(currentMillis);
    //   PowerBooted = true;
    Serial.print("POWERED on");
    power_cell_idle();



    /* DO NOT TOUCH ABOVE THIS LINE
      }


      if (!digitalRead(STARTWAND_SWITCH) && (poweroff == false)) {

      wand_on = true;
      if ((startpack)&& (poweroff == false)) {

      mp3.playTrack(1);// wand on sound
      WANDledstate2();      // turns wand leds on
      startpack = false;
      }
      // code here
      }
      else {
      startpack = true;
      if (wand_on) {
      mp3.playTrack(1); // wand sound
      wand_on = false;
      }
      }




      /* if (packidle == true){
      /******************************************** HERE IS THE MAGIC/IDLE/ begin reading STAGE*******************************************************************
          setWandLightState(12, 0, 0);    // sloblo red
           setWandLightState(19, 6, currentMillis); // Set orange hat barrel flashing
           setWandLightState(18, 9, 0); // set back led green

          barGraphSequenceOne(currentMillis);
          power_cell_idle();
           mp3.playTrack(10);
      } else

      packidle == false;

         /***********************************************************************************************************************************

      // need this track  ( 8 ) to play for 12 seconds
      // before moving onto the idle stage but not interfering with the bargraph charging or charging sequence from the wand
      }





      /* here we need to read these in order to enable heat pin to work and to start the warning and venting procedure

        read STARTPACK_SWITCH - if condition is met  -  run idle stage
       read STARTWAND_SWITCH - if condition is met  -  run idle stage - read >>(safety one)
                               if not, do nothing.
                 SAFETY_ONE -  if condition is met, -  run safety1_read - read >> (safety two)
                               if not, return to idle stage.
                 SAFETY_TWO -  if condition is met - run safety2_read - read >> (HEAT_PIN)
                               if not, return to safety one.
                  HEAT_PIN  -  if condition is met - run fire_seq >> (warning procedure)
                               if not, return to safety two.
        (warning procedure) -  if HEAT_PIN is pressed for 15 seconds -
                               enter warning stage
                               warning stage for 15 seconds
                               vent stage
                               return to safety2_read

      poweroff = false;            //false
      if (PowerBooted == false) {
      PowerBooted = true;             //true
      startpack = true;              // true

      // turn on wand

      if (!digitalRead(STARTWAND_SWITCH)&& (startpack == true) ) {
      if (wand_on == false) {
      mp3.playTrack(1);     // wand on sound
      wand_on = true;
      WANDledstate2();      // turns wand leds on
      Serial.print("wand enabled");
      //   safety1_read();        // read the first safety switch

      }
      else
      mp3.playTrack(13);
      wand_on = false;




      ------------------------------------------------






    */
    // -------------------------  turn pack off ----------------------------------------------//


    if (digitalRead(STARTPACK_SWITCH) && (poweroff == false) ) {
        if (poweroff)
        poweroff = true;
      Serial.print("POWERED DOWN");

      mp3.playTrack(9);


      //PowerBooted = false;
      //startpack = false;
      
      cyclotron.setBrightness(0, shutoff_DLY);
      lightsout();        //powercell lights off
      shutdown_leds();    // bargraph off
      WANDLEDstateOFF();   // wand lights off
      mp3.playPause();
    }
  }
}
