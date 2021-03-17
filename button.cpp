#include "Arduino.h"
#include "button.h"


Button::Button(uint8_t pins[10], uint8_t count, unsigned int interval) : 
    lastDebounceTime(0),
    debounceDelay(interval),
    button_count(count),
    button_pins(pins),
    lastButtonState(LOW)
{
    
}

void Button::Setup(){
  uint8_t i;
  for (i = 0 ; i < button_count; i++){
    pinMode(button_pins[i], INPUT_PULLUP);
  }
}

void Button::handleButtons(){
  int i;
  int button_pressed;
  int reading;
  for (i = 0; i < button_count; i++){
    if (reading = !digitalRead(button_pins[i])){
      button_pressed = button_pins[i];
      break;
    }
  }
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      
      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        if (callback)
          callback(button_pressed);
      }
    }
  }
  
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  
}

void Button::setCallback(void (*callback_func)(uint8_t button)){
  callback = callback_func;
}
