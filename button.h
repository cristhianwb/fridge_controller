
#ifndef BUTTON_H
#define BUTTON_H

class Button {
  public:
    Button(uint8_t pins[10], uint8_t count,unsigned int interval);
    //Callback function to call when button a is pressed
    void setCallback(void (*callback_func)(uint8_t));
    void handleButtons();
    void Setup();
  private:
    uint8_t* button_pins;
    uint8_t button_states[10];
    int lastButtonState;
    int buttonState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;
    uint8_t button_count;
    void (*callback)(uint8_t);   
    
};

#endif
