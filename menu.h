#ifndef MENU_H
#define MENU_H

#include <LiquidCrystal.h>

typedef enum MENU_TYPE_ {MN_FLOAT = 1, MN_STRING = 2, MN_INT = 3, MN_SUBMENU = 4} MENU_TYPE;
typedef enum Action_ {BT_UP, BT_DOWN, BT_LEFT, BT_RIGHT} Action; 




typedef struct _menu menu;

typedef union _MenuValue{
  int i;
  float f;
  menu ** submenu;
}MenuValue;

typedef struct _menu{
  const char * desc; //Menu description
  MenuValue val; //Default value
  MENU_TYPE menu_type; //Menu type
  uint16_t eeprom_addr; //Value address in EEPROM (used to save the new value)
}menu;





class MenuRender{
  public:
    MenuRender(menu** menus, LiquidCrystal* screen);
    Render();
    doAction(Action action);
  private:
    const char **const options_str;
    menu** menus;
    LiquidCrystal* screen;    
  
};










#endif
