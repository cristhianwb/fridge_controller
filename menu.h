#ifndef MENU_H
#define MENU_H

typedef enum MENU_TYPE_ {MN_FLOAT = 1, MN_STRING = 2, MN_INT = 3} MENU_TYPE;
typedef enum Action_ {BT_UP, BT_DOWN, BT_LEFT, BT_RIGHT} Action; 

typedef struct _menu{
  void *val;
  MENU_TYPE menu_type;
}menu;


class MenuRender{
  public:
    MenuRender(const char *const options_str[], menu* menus, LiquidCrystal screen);
    Render();
    doAction(Action action);
  private:
    
  
};










#endif
