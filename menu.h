#ifndef MENU_H
#define MENU_H

#include <LiquidCrystal.h>
#include "button.h"

//The maximum number of submenus
#define MAX_MENU_STACK 5

typedef enum MENU_TYPE_ {MN_FLOAT = 1, MN_STRING = 2, MN_INT = 3, MN_SUBMENU = 4, MN_VIEW} MENU_TYPE;
typedef enum State_ {ST_SET_MENU, ST_SET_INT, ST_SET_FLOAT, ST_SET_STR, ST_SET_ON_OFF} State;

typedef struct _menu menu;

typedef union _MenuValue{
  int i;
  float f;
  menu ** submenu;
}MenuValue;

typedef struct _menu{
  const char * desc; //Menu description
  void* val; //Default value
  MENU_TYPE menu_type; //Menu type
  uint16_t eeprom_addr; //Value address in EEPROM (used to save the new value)
  void (*callback)(MenuValue);
};

class MenuHandler{
  public:
    MenuHandler(menu** menus, LiquidCrystal* screen);
    void Render();
    void buttonClick(uint8_t btn);
  private:
    const char **const options_str;
    menu** menus;
    menu** menu_stack[5];
    int menu_stack_count;
    menu cur_menu;
    LiquidCrystal* screen;
    uint8_t col, row;
    State cur_state;
    void load_cur_menu(int8_t step);
    void handle_set_menu(uint8_t btn);
    void handle_set_int(uint8_t btn);   
  
};

#endif
