#include "menu.h"


MenuHandler::MenuHandler(menu** menus, LiquidCrystal* screen): 
          options_str(options_str),
          menus(menus),
          cur_state(ST_SET_MENU),
          screen(screen),
          col(0),
          row(0){
  //Procura o primeiro menu não vazio
  for (;pgm_read_word(this->menus) == NULL; this->menus++);
  load_cur_menu(0);
  memset(this->menu_stack, NULL, MAX_MENU_STACK);
  this->menu_stack_count = 0;
}

void MenuHandler::load_cur_menu(int8_t step){
  if ((step != 0) && (pgm_read_word(this->menus+step) != NULL))
    this->menus += step;
  memcpy_P(&this->cur_menu, pgm_read_word(this->menus), sizeof(menu));
}

void MenuHandler::handle_set_menu(uint8_t btn){
  switch (btn){
    case BT_UP:
      load_cur_menu(-1);
    break;
    
    case BT_DOWN:
      load_cur_menu(1);
    break;
        
    case BT_LEFT:
      if (this->menu_stack_count > 0){
        this->menus = this->menu_stack[--this->menu_stack_count];
        this->load_cur_menu(0);         
      }
    break;

    case BT_RIGHT:
      switch(this->cur_menu.menu_type){
        case MN_INT:
          this->cur_state = ST_SET_INT;
        break;

        case MN_SUBMENU:
          if (this->menu_stack_count+1 < MAX_MENU_STACK){
            this->menu_stack[this->menu_stack_count++] = this->menus;
            this->menus = (menu**) this->cur_menu.val;
            for (;pgm_read_word(this->menus) == NULL; this->menus++);
            this->load_cur_menu(0);
          }
        break;
      }
    break;
  }
}

void MenuHandler::handle_set_int(uint8_t btn){
  switch (btn){
    case BT_UP:

    break;
    case BT_DOWN:

    break;
    
    case BT_LEFT:

    break;

    case BT_RIGHT:

    break;
  }
}
/*
when clicking a button, the action should be redirected to the handler of the current state in state machine
the handler then you tell if should change de state or not
*/          
void MenuHandler::buttonClick(uint8_t btn){
  switch (this->cur_state){
    case ST_SET_MENU:
      handle_set_menu(btn);
    break;
        
    case ST_SET_INT:

    break;
  }
  this->Render();
}

void MenuHandler::Render(){
  char text[20];
  strcpy_P(text, this->cur_menu.desc);
  //itoa(this->cur_menu.val.i, text, 10);
  screen->clear();
  screen->setCursor(0, 0);
  col += screen->print(text);
  
  if (this->cur_state == ST_SET_MENU)
    screen->noCursor();
  else
    screen->cursor();
  
}
