/////////////////////////////////////////////////////////////////
#pragma once

#ifndef Button3_h
#define Button3_h

/////////////////////////////////////////////////////////////////

#include "Arduino.h"

/////////////////////////////////////////////////////////////////

#define DEBOUNCE_MS      50
#define LONGCLICK_MS    500
#define DOUBLECLICK_MS  800

#define SINGLE_CLICK      1
#define DOUBLE_CLICK      2
#define TRIPLE_CLICK      3
#define QUATLE_CLICK      4
#define QUINTLE_CLICK     5
#define CONFIG_CLICK      6
#define LONG_CLICK        7

/////////////////////////////////////////////////////////////////

class Button3 {
  private:
    byte pin;
    int prev_state;
    int state = HIGH;
    byte click_count = 0;
    unsigned int last_click_type = 0;
    unsigned long click_ms;
    unsigned long down_ms;
    unsigned int debounce_time_ms;
    unsigned int down_time_ms = 0;
    bool pressed_triggered = false;
    bool longclick_detected = false;
        
    typedef void (*CallbackFunction) (Button3&);

    CallbackFunction pressed_cb = NULL;
    CallbackFunction released_cb = NULL;
    CallbackFunction change_cb = NULL;
    CallbackFunction tap_cb = NULL;
    CallbackFunction click_cb = NULL;
    CallbackFunction long_cb = NULL;
    CallbackFunction double_cb = NULL;
    CallbackFunction triple_cb = NULL;
	CallbackFunction quatle_cb = NULL;
	CallbackFunction quintle_cb = NULL;
	CallbackFunction config_cb = NULL;
    
  public:
    Button3(byte attachTo, byte buttonMode = INPUT_PULLUP, unsigned int debounceTimeout = DEBOUNCE_MS);
    void setDebounceTime(unsigned int ms);
    
    void setChangedHandler(CallbackFunction f);
    void setPressedHandler(CallbackFunction f);
    void setReleasedHandler(CallbackFunction f);
    void setClickHandler(CallbackFunction f);
    void setTapHandler(CallbackFunction f);
    void setLongClickHandler(CallbackFunction f);
    void setDoubleClickHandler(CallbackFunction f);
    void setTripleClickHandler(CallbackFunction f);
	void setQuatleClickHandler(CallbackFunction f);
	void setQuintleClickHandler(CallbackFunction f);
	void setConfigClickHandler(CallbackFunction f);

    unsigned int wasPressedFor();
    boolean isPressed();

    unsigned int getNumberOfClicks();
    unsigned int getClickType();
    
    bool operator==(Button3 &rhs);

    void loop();
};
/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
