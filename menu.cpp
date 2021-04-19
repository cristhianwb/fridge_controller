#include "menu.h"


MenuHandler::MenuHandler(menu** menus, LiquidCrystal* screen, const char* atitle): 
          menus(menus),
          cur_state(ST_SET_MENU),
          screen(screen),
          col(0),
          row(0){
  //Finds the first not null menu, the menu list has a NULL pointer in the first
  //and last postions of the vector, that serves as a flag
  for (;pgm_read_word(this->menus) == NULL; this->menus++);
  load_cur_menu(0);
  memset(this->menu_stack, NULL, MAX_MENU_STACK);
  this->menu_stack_count = 0;
  main_title = atitle;
  strncpy(title, main_title, 10);
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
      //When clicking button left, the previous menu will be popped from the menu stack
      //thus returning to the previous menu
      if (this->menu_stack_count > 0){
        this->menus = this->menu_stack[--this->menu_stack_count];
        this->load_cur_menu(0);
        if (this->menu_stack_count == 0)
          strncpy(title, main_title, 10);
        /*else{}
         * when implementing more than one submenu levels,
         * need to copy the parent menu title
        */           
      }
    break;
    
    case BT_RIGHT:
      //The action when clicknig right will depend on the type of the menu
      //this is treated right here
      switch(this->cur_menu.menu_type){
        case MN_INT:
          this->cur_state = ST_SET_INT;
        break;

        case MN_ON_OFF:
          this->cur_state = ST_SET_ON_OFF;
        break;

        case MN_FLOAT:
          this->cur_state = ST_SET_FLOAT;
          col = f_point-1;
          f_set_dec = true;
        break;

        case MN_STRING:
          this->cur_state = ST_SET_STR;
        break;

        case MN_SUBMENU:
          //When entering a submenu the current menu should be pushed to the submenu stack
          //so that you can return to the previous menu
          if (this->menu_stack_count+1 < MAX_MENU_STACK){
            strcpy_P(title, this->cur_menu.desc);
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
  int* val;
  val = (int*) this->cur_menu.val;
  switch (btn){
    case BT_UP:
      (*val)++;
    break;
    case BT_DOWN:
      (*val)--;
    break;
    
    case BT_LEFT:
      this->cur_state = ST_SET_MENU;
      col = 0;
      return;
    break;

    case BT_RIGHT:

    break;
  }
  
}

void MenuHandler::handle_set_float(uint8_t btn){
    float* val;
    val = (float*) this->cur_menu.val;

        
    switch (btn){
    case BT_UP:
      if (f_set_dec)
        *val+=1;
      else
        *val+=0.01;
    break;
    
    case BT_DOWN:
      if (f_set_dec)
        *val-=1;
      else
        *val-=0.01;
    break;
    
    case BT_LEFT:
      if (f_set_dec){
        this->cur_state = ST_SET_MENU;
        col = 0;
        return;
      }
      else
        f_set_dec = true;        
    break;

    case BT_RIGHT:
      f_set_dec = false;
    break;

  }

}

void MenuHandler::handle_set_string(uint8_t btn){
  
}

void MenuHandler::handle_set_on_off(uint8_t btn){
    bool* val;
    val = (bool*) this->cur_menu.val;
    
    switch (btn){
      case BT_UP:
        (*val) = !(*val);    
      break;
      
      case BT_DOWN:
        (*val) = !(*val);
      break;
      
      case BT_LEFT:
        this->cur_state = ST_SET_MENU;
        col = 0;
          return;
      break;
  
    }

}

/*
when clicking a button, the action should be redirected to the handler of the current state in state machine
the handler then will tell if should change the state or not
*/          
void MenuHandler::buttonClick(uint8_t btn){
  switch (this->cur_state){
    case ST_SET_MENU:
      handle_set_menu(btn);
    break;
        
    case ST_SET_INT:
      handle_set_int(btn);  
    break;

    case ST_SET_FLOAT:
      handle_set_float(btn);
    break;

    case ST_SET_STR:
      handle_set_string(btn);
    break;

    case ST_SET_ON_OFF:
      handle_set_on_off(btn);
    break;
    
  }
  if ((this->cur_menu.menu_type != MN_VIEW) && (this->cur_menu.callback != NULL))
      this->cur_menu.callback(this->cur_menu.val);
  this->Render();
}


int MenuHandler::print_menu(char *text){
  char desc[14];
  char * val;
  int sz, i;
  strcpy_P(desc, this->cur_menu.desc);
  sz = sprintf(text, "> %s%s ", desc, (this->cur_menu.menu_type) == MN_SUBMENU ? ">>" : ":");
  switch(this->cur_menu.menu_type){
    case MN_INT:
      col = sprintf(text+sz, "%d", *((int*) this->cur_menu.val))-1;
    break;

    case MN_FLOAT:
      val = dtostrf( *((float*) this->cur_menu.val), 2, 2, text+sz);
      for (f_point = -1,i = 0;val[i] != 0; i++)
        if ( val[i] == '.' ){
          f_point = i;
          break;
        }
        
        if (f_set_dec)
          col = f_point-1;
        else
          col = f_point+2;
          
    break;

    case MN_ON_OFF:
      col = sprintf(text+sz, "%s", *((bool*) this->cur_menu.val) ? "ON" : "OFF")-1;
    break;
  }
  return sz;
}

void MenuHandler::Render(){
  char text[32];
  int desc_size;
  text[0] = 0;
  text[16] = 0;
  
  if (this->cur_menu.menu_type == MN_VIEW){
    this->cur_menu.callback(text);
  }else{
     sprintf(text, "== %s ==", title);
     desc_size = print_menu(text+16);
  }
  
  screen->clear();
  screen->setCursor(0, 0);
  screen->print(text);
  screen->setCursor(0, 1);
  screen->print(&text[16]);
  
  if (this->cur_state == ST_SET_MENU)
    screen->noCursor();
  else{
    screen->cursor();
    screen->setCursor(desc_size + col, 1);  
  }
}
