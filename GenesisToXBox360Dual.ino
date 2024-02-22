#include <libmaple/iwdg.h>
#include "SegaController.h"
#include <USBComposite.h>

#define USB_DISCONNECT_DELAY 500 // for some devices, may want something smaller
#define LED PC13

#define START_DEPRESSION_TIME 200

const uint32_t watchdogSeconds = 6;

//Note: Do not use pins PA11 and PA12. They are reserverd.
// Looking straight on at female socket (or from back of male jack):

// 5 4 3 2 1
//  9 8 7 6

// 3.3 PB15 PB14 PB13 PB12
//   PA9 GND PA10 PA8

// 3.3 PA3 PA2 PA1 PA0
//   PA6 GND PA5 PA4

// pins:                    7,   1,   2,    3,    4,    6,   9
SegaController sega(      PA10, PB12, PB13, PB14, PB15, PA8, PA9);
SegaController segaSecond(PA5,  PA0,  PA1,  PA2,  PA3,  PA4, PA6);

#define GUIDE_PIN PB7
#define START_PIN PB8
#define BACK_PIN PB9

#define NUM_INPUTS 2

SegaController* inputs[] = { &sega, &segaSecond };

USBXBox360W<2> XBox360;

uint32_t lastDataTime[2] = { 0, 0 };

struct start_data {
  uint32 time;
  boolean pressed;
  boolean combo;
} start[2] = { {0} };

/**
 *  SC_CTL_ON    = 1, // The controller is connected
 *  SC_BTN_UP    = 2,
 *  SC_BTN_DOWN  = 4,
 *  SC_BTN_LEFT  = 8,
 *  SC_BTN_RIGHT = 16,
 *  SC_BTN_START = 32,
 *  SC_BTN_A     = 64,
 *  SC_BTN_B     = 128,
 *  SC_BTN_C     = 256,
 *  SC_BTN_X     = 512,
 *  SC_BTN_Y     = 1024,
 *  SC_BTN_Z     = 2048,
 *  SC_BTN_MODE  = 4096,
 */

const uint16_t remap_xbox[16] = {
  0xFFFF,         //CTL_ON
  XBOX_DUP,       //UP
  XBOX_DDOWN,     //DOWN
  XBOX_DLEFT,     //LEFT
  XBOX_DRIGHT,    //RIGHT
  XBOX_START,     //BTN_START
  XBOX_X,         //A
  XBOX_A,         //B
  XBOX_B,         //C
  XBOX_LSHOULDER, //X
  XBOX_Y,         //Y
  XBOX_RSHOULDER, //Z
  XBOX_GUIDE,     //MODE
  0xFFFF,
  0xFFFF,
  0xFFFF
};

const uint16_t* remap = remap_xbox;

void reset(USBXBox360WController* c) {
  c->X(0);
  c->Y(0);
  c->buttons(0);
}

void setup() {
  iwdg_init(IWDG_PRE_256, watchdogSeconds*156);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,1);
  USBComposite.setProductString("GenesisToUSB");
  USBComposite.setDisconnectDelay(USB_DISCONNECT_DELAY);
  XBox360.begin();
  for (uint8 n = 0 ; n < 2 ; n++) {
    USBXBox360WController* c = &XBox360.controllers[n];
    c->setManualReportMode(true);
  }

  pinMode(GUIDE_PIN, INPUT_PULLUP);
  pinMode(START_PIN, INPUT_PULLUP);
  pinMode(BACK_PIN, INPUT_PULLUP);
}

void loop() {
  iwdg_feed();

  if (! USBComposite)
    return;

  bool active = false;

  for (uint32 n = 0 ; n < NUM_INPUTS ; n++) {
    word state = inputs[n]->getState();
    USBXBox360WController* c = &XBox360.controllers[n];

    if (state & SC_CTL_ON) {  //controller connected
      lastDataTime[n] = millis();
      active = true;

      c->buttons(0);

      //wire up extra button pins, only trigger on first controller
      if (n == 0) {
        if (digitalRead(GUIDE_PIN) == LOW) { c->button(XBOX_GUIDE, 1); }
        if (digitalRead(START_PIN) == LOW) { c->button(XBOX_START, 1); }
        if (digitalRead(BACK_PIN) == LOW) { c->button(XBOX_BACK, 1); }
      }

      struct start_data *s = start + n;
      if (state & SC_BTN_START) { //start button held down
        s->pressed = true;
        s->time = millis();
        int16 x = 0;
        int16 y = 0;

        if (state & SC_BTN_LEFT) {
          x = -32768;
          s->combo = true;
        }
        else if (state & SC_BTN_RIGHT) {
          x = 32767;
          s->combo = true;
        }

        if (state & SC_BTN_UP) {
          y = 32767;
          s->combo = true;
        }
        else if (state & SC_BTN_DOWN){
          y = -32768;
          s->combo = true;
        }

        c->X(x);
        c->Y(y);

        if (state & SC_BTN_A) {
          c->button(XBOX_LSHOULDER, 1);
          s->combo = true;
        }
        if (state & SC_BTN_B) {
          c->button(XBOX_GUIDE, 1);
          s->combo = true;
        }
        if (state & SC_BTN_C) {
          c->button(XBOX_RSHOULDER, 1);
          s->combo = true;
        }

        if (state & SC_BTN_X) {
          c->button(XBOX_BACK, 1);
          s->combo = true;
        }
        if (state & SC_BTN_Y) {
          c->button(XBOX_GUIDE, 1);
          s->combo = true;
        }
        if (state & SC_BTN_Z) {
          c->button(XBOX_R3, 1);
          s->combo = true;
        }
      }
      else {  //start button not held down
        uint16_t mask = 1;
        for (int i = 0; i < 16; i++, mask <<= 1) {
          uint16_t xb = remap[i];
          if (xb != 0xFFFF && (state & mask))
          c->button(xb, 1);
        }

        //check if start button released
        uint32 t = millis();
        if (s->pressed) {
          if (s->combo)
            s->time = 0;
          else
            s->time = t;
        }
        if (s->time != 0 && t <= s->time + START_DEPRESSION_TIME)
          c->button(XBOX_START, 1);
        else
          s->time = 0;
        s->pressed = false;
        s->combo = false;
      }
      c->send();
    }
    else if (c->isConnected() && millis() - lastDataTime[n] >= 2000) {
       // we hold the last state for 2 seconds, in case something's temporarily wrong with the transmission
       // but then we just clear the data
       reset(c);
       c->send();
       c->connect(false);
    }
  }
  digitalWrite(LED,active?0:1);
}
