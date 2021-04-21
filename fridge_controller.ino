 
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

struct {
  double Kp = 5, Ki = 0.05, Kd = 0.2;  
  double SetPoint = 15;
  int contrast = 30;
  int relay_threshould_on = 30;
  bool motor_on = false;
}cfg;


double tempC = 0;
int output = 0;
bool save = false, areset = false;
bool relay = false;



//The submenus of PID menu
const char PID_mn_desc[] PROGMEM = "PID Conf.";
const char P_mn_desc[] PROGMEM = "Prop.";
const char I_mn_desc[] PROGMEM = "Int.";
const char D_mn_desc[] PROGMEM = "Der.";
const char SetP_mn_desc[] PROGMEM = "Set Po.";
const char relay_mn_desc[] PROGMEM = "Rly Trh.";
//The main menus

const char cont_mn_desc[] PROGMEM = "Contrst.";
const char motor_on_mn_desc[] PROGMEM = "Motor";

const char gen_mn_desc[] PROGMEM = "Cfg. Ger.";

const char save_mn_desc[] PROGMEM = "Save?";
const char reset_mn_desc[] PROGMEM = "Reset?";

void set_PID(void* val);

void temp_status(void* val);

void call_reset(void* val);

void call_save(void* val);


//Aqui são definidos os menus

//Primeiro menu, status atual
const menu Temp_menu PROGMEM = {.desc = NULL,.val = &tempC, .menu_type = MN_VIEW,  .callback = temp_status};

//Segundo menu, menu de configurações do PID
//Primeiro os submenus
const menu P_menu PROGMEM = {.desc = P_mn_desc,.val = &cfg.Kp, .menu_type = MN_FLOAT, .callback = set_PID};
const menu I_menu PROGMEM = {.desc = I_mn_desc,.val = &cfg.Ki, .menu_type = MN_FLOAT, .callback = set_PID};
const menu D_menu PROGMEM = {.desc = D_mn_desc,.val = &cfg.Kd, .menu_type = MN_FLOAT, .callback = set_PID};
const menu StP_menu PROGMEM = {.desc = SetP_mn_desc,.val = &cfg.SetPoint, .menu_type = MN_FLOAT, .callback = set_PID};
const menu thr_on_menu PROGMEM = {.desc = relay_mn_desc,.val = &cfg.relay_threshould_on, .menu_type = MN_INT, .callback = NULL};
//E depois o menu principal
const menu *const pid_options[] PROGMEM = {NULL, &P_menu, &I_menu, &D_menu, &StP_menu, &thr_on_menu, NULL};
const menu Pid_menu PROGMEM = {.desc = PID_mn_desc,.val = pid_options, .menu_type = MN_SUBMENU };

//Por ultimo o menu de configurações gerais
//Primeiro os submenus
const menu save_menu PROGMEM = {.desc = save_mn_desc,.val = &save, .menu_type = MN_ON_OFF,.callback = call_save};
const menu reset_menu PROGMEM = {.desc = reset_mn_desc,.val = &areset, .menu_type = MN_ON_OFF, .callback = call_reset };
const menu contrast_menu PROGMEM = {.desc = cont_mn_desc,.val = &cfg.contrast, .menu_type = MN_INT };
const menu motor_on PROGMEM = {.desc = motor_on_mn_desc,.val = &cfg.motor_on, .menu_type = MN_ON_OFF };

//Depois o principal
const menu *const gen_options[] PROGMEM = {NULL,&motor_on, &contrast_menu, &save_menu, &reset_menu,  NULL};
const menu Gen_conf_menu PROGMEM = {.desc = gen_mn_desc,.val = gen_options, .menu_type = MN_SUBMENU };

//Aqui é definida a lista principal de menus 
//Foi adicionado um elemnto NULL no inicio e no fim, para indicar onde começa e termina a lista
const menu *const options[] PROGMEM = {NULL, &Temp_menu, &Pid_menu, &Gen_conf_menu, NULL};

 
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

PID_v2 myPID(cfg.Kp, cfg.Ki, cfg.Kd, PID::Reverse);


void call_reset(void* val){
  if (*((bool*) val)){
    EEPROM.write(0, 0xFF);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Resseted.");
    lcd.setCursor(0,1);
    lcd.print("Restart!");
    delay(1000);
    *((bool*) val) = false;    
  }
}

void call_save(void* val){
  if (*((bool*) val)){
    save_cfg();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Saved!");
    delay(1000);
    *((bool*) val) = false;
  }
}

void set_PID(void* val){
  myPID.SetTunings(cfg.Kp, cfg.Ki, cfg.Kd);
  myPID.Setpoint(cfg.SetPoint);
}

void temp_status(void* val){
  char t_str[6];
  dtostrf(tempC, 2, 2, t_str);
  sprintf( (char*) val, "T: %s M: %s", t_str, relay ? "ON": "OFF");
  dtostrf(cfg.SetPoint, 2, 2, t_str);
  sprintf((char*) val+16, "S: %s P: %d",t_str, output);
}


void save_cfg(){
  int eeprom_addr = 0, i;
  EEPROM.update(eeprom_addr++, 0xAA);  
  EEPROM.put(eeprom_addr, cfg);
  
}

void load_cfg(){
  int eeprom_addr = 0, i;
  
  
  if (EEPROM.read(eeprom_addr++) == 0xFF){
    save_cfg();
    return;
  }
  EEPROM.get(eeprom_addr, cfg);  
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
  pinMode(PIN_A1, OUTPUT);
  sensors.requestTemperatures();
  delay(800);
  sensors.requestTemperatures();
  delay(800);
  tempC = sensors.getTempC(sensor1);
  load_cfg();
  set_PID(NULL);
  myPID.SetSampleTime(1000);
  myPID.Start(tempC,  // input
              0,                      // current output
              cfg.SetPoint);

}

 
void loop(){
  buttons.handleButtons();
  

  unsigned long curtime = millis();
  if( ((curtime - lastDspUpdte) > 1000)){
    lastDspUpdte = curtime;
    tempC = sensors.getTempC(sensor1);
    sensors.requestTemperatures(); 
    menu_handler.Render();
    output = (int) myPID.Run(tempC);
    relay = (output > cfg.relay_threshould_on) && cfg.motor_on;
    analogWrite(3, output);
    digitalWrite(PIN_A1, relay);
    analogWrite(11, cfg.contrast);    
  }  
}
