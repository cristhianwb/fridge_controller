 
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>
#include <PID_v2.h>
#include <EEPROM.h>
#include "menu.h"

#include "button.h"
#include <avr/pgmspace.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define NUM_STR_SZ 5
#define NUM_SPR_POS 2

#define OPTION_COUNT 7

double Kp = 2, Ki = 5, Kd = 1;
float tempC = 0;
int contrast = 30;

//The submenus of PID menu
const char P_mn_desc[] PROGMEM = "Propor.";
const char I_mn_desc[] PROGMEM = "Integr.";
const char D_mn_desc[] PROGMEM = "Deriv.";
//The main menus
const char PID_mn_desc[] PROGMEM = "PID Conf.";
const char cont_mn_desc[] PROGMEM = "Contrst.";
const char relay_mn_desc[] PROGMEM = "Rlay Conf.";

const char relay_on_mn_desc[] PROGMEM = "ON Trh.";
const char relay_off_mn_desc[] PROGMEM = "OFF Thr.";

void set_PID(void* val);

void temp_status(void* val);


//Aqui são definidos os menus

const menu P_menu PROGMEM = {.desc = P_mn_desc,.val = &Kp, .menu_type = MN_FLOAT, .eeprom_addr = NULL, .callback = set_PID};
const menu I_menu PROGMEM = {.desc = I_mn_desc,.val = &Ki, .menu_type = MN_FLOAT, .eeprom_addr = NULL, .callback = set_PID};
const menu D_menu PROGMEM = {.desc = D_mn_desc,.val = &Kd, .menu_type = MN_FLOAT, .eeprom_addr = NULL, .callback = set_PID};

const menu thr_on_menu PROGMEM = {.desc = relay_on_mn_desc,.val = NULL, .menu_type = MN_INT, .eeprom_addr = NULL, .callback = NULL};
const menu thr_off_menu PROGMEM = {.desc = relay_off_mn_desc,.val = NULL, .menu_type = MN_INT, .eeprom_addr = NULL, .callback = NULL};


const menu *const pid_options[] PROGMEM = {NULL, &P_menu, &I_menu, &D_menu, NULL};
const menu *const relay_options[] PROGMEM = {NULL, &thr_on_menu, &thr_off_menu,  NULL};

const menu Temp_menu PROGMEM = {.desc = NULL,.val = &tempC, .menu_type = MN_VIEW, .eeprom_addr = NULL, .callback = temp_status};
//Nesse caso como é um submenu o valor dele aponta para um array de menus
//Quando entrar nele o MenuHandler vai precisar gravar a referencia do array de menus anterior para retornar
const menu Pid_menu PROGMEM = {.desc = PID_mn_desc,.val = pid_options, .menu_type = MN_SUBMENU, .eeprom_addr = NULL};
const menu Relay_conf_menu PROGMEM = {.desc = relay_mn_desc,.val = relay_options, .menu_type = MN_SUBMENU, .eeprom_addr = NULL};
const menu contrast_menu PROGMEM = {.desc = cont_mn_desc,.val = &contrast, .menu_type = MN_INT, .eeprom_addr = NULL};

//Aqui é definida a lista de menus
//Foi adicionado um elemnto NULL no inicio e no fim, para indicar onde começa e termina a lista
const menu *const options[] PROGMEM = {NULL, &Temp_menu, &Pid_menu, &Relay_conf_menu, &contrast_menu, NULL};


 
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(7, 6, 5, 4, A5, 2);
uint8_t btns[] = {BT_DOWN, BT_UP, BT_LEFT, BT_RIGHT};
Button buttons(btns, 4, 50);
MenuHandler menu_handler(options, &lcd, "Config.");

#define ONE_WIRE_BUS 14

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;


unsigned long lastDspUpdte = 0;
int ledState = LOW;
int pinpressed;

//MENU DEFS
uint8_t cur_line = 0;
uint8_t cur_col = 0;
uint8_t field_pos = 0;


PID_v2 myPID(Kp, Ki, Kd, PID::Direct);


void set_PID(void* val){
  
}

void temp_status(void* val){
  char t_str[6];
  dtostrf(tempC, 2, 2, t_str);
  sprintf( (char*) val, "Temp.: %s", t_str);
  sprintf((char*) val+16, "Hey");
}

void callback(uint8_t btn){
  menu_handler.buttonClick(btn);
}
 
void setup()
{
  //pinMode(A7, OUTPUT);
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  lcd.cursor();

  //Setup of the debounce algo
  //pinMode(buttonPin, INPUT_PULLUP);
  //pinMode(ledPin, OUTPUT);
  buttons.Setup();
  buttons.setCallback(callback);
  sensors.getAddress(sensor1, 0);
  pinMode(3, OUTPUT);
  pinMode(11, OUTPUT);
     
}

 
void loop(){
  buttons.handleButtons();
  analogWrite(11, contrast);
  unsigned long curtime = millis();
  if( ((curtime - lastDspUpdte) > 1000 && (cur_line == 0) )){
    lastDspUpdte = curtime;
    sensors.requestTemperatures();
    tempC = sensors.getTempC(sensor1); 
    menu_handler.Render();
    //print_status();
  }
  
  
}
