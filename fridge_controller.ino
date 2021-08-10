 
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
  double cooling_sp = 18, heating_sp = 16;
  double cooling_hyst, heating_hyst;
  int cooling_f_speed = 0, heating_f_speed = 0; 
  bool cooling_en = true, heating_en = false;
  int contrast = 30;
  int relay_threshould_on = 30;
  bool motor_on = false;
  
}cfg;


double tempC = 0;
int fan = 0;
bool cooling = false, heating = false;
bool save = false, areset = false;
bool relay = false;


/******* Title Strings of menus **********/


/*### Titles of cooling menu ###*/

const char cooling_mn_desc[] PROGMEM = "Cooling Cfg";


/*### Titles of the heating menu*/

const char heating_mn_desc[] PROGMEM = "Heating Cfg";
const char heating_hyst_desc[] PROGMEM = "Hysteresis";

//Common to both
const char SetP_mn_desc[] PROGMEM = "Set Po.";
const char fan_desc[] PROGMEM = "Fan Speed";
const char hyst_desc[] PROGMEM = "Hyster.";
const char en_desc[] PROGMEM = "Enabled";

//The main config menus

const char gen_mn_desc[] PROGMEM = "Main Cfg.";
const char cont_mn_desc[] PROGMEM = "Contrst.";
const char motor_on_mn_desc[] PROGMEM = "Motor";
const char save_mn_desc[] PROGMEM = "Save?";
const char reset_mn_desc[] PROGMEM = "Reset?";

void set_PID(void* val);

void temp_status(void* val);

void call_reset(void* val);

void call_save(void* val);


//Here the menus are defined

//First of all, the status menu
const menu Temp_menu PROGMEM = {.desc = NULL,.val = &tempC, .menu_type = MN_VIEW,  .callback = temp_status};

//Second one, cooling menus

const menu cooling_hyst_menu PROGMEM = {.desc = hyst_desc,.val = &cfg.cooling_hyst, .menu_type = MN_FLOAT, .callback = set_PID};
const menu cooling_fan_menu PROGMEM = {.desc = fan_desc,.val = &cfg.cooling_f_speed, .menu_type = MN_INT, .callback = set_PID};
const menu cooling_StP_menu PROGMEM = {.desc = SetP_mn_desc,.val = &cfg.cooling_sp, .menu_type = MN_FLOAT, .callback = set_PID};
const menu cooling_en_menu PROGMEM = {.desc = en_desc,.val = &cfg.cooling_en, .menu_type = MN_ON_OFF, .callback = set_PID};

const menu *const cooling_options[] PROGMEM = {NULL, &cooling_hyst_menu, &cooling_fan_menu, &cooling_StP_menu, &cooling_en_menu, NULL};

const menu cooling_menu PROGMEM = {.desc = cooling_mn_desc,.val = cooling_options, .menu_type = MN_SUBMENU };



const menu heating_hyst_menu PROGMEM = {.desc = hyst_desc,.val = &cfg.heating_hyst, .menu_type = MN_FLOAT, .callback = set_PID};
const menu heating_fan_menu PROGMEM = {.desc = fan_desc,.val = &cfg.heating_f_speed, .menu_type = MN_INT, .callback = set_PID};
const menu heating_StP_menu PROGMEM = {.desc = SetP_mn_desc,.val = &cfg.heating_sp, .menu_type = MN_FLOAT, .callback = set_PID};
const menu heating_en_menu PROGMEM = {.desc = en_desc,.val = &cfg.heating_en, .menu_type = MN_ON_OFF, .callback = set_PID};

const menu *const heating_options[] PROGMEM = {NULL, &heating_hyst_menu, &heating_fan_menu, &heating_StP_menu, &heating_en_menu, NULL};

const menu heating_menu PROGMEM = {.desc = heating_mn_desc,.val = heating_options, .menu_type = MN_SUBMENU };


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
const menu *const options[] PROGMEM = {NULL, &Temp_menu, &cooling_menu, &heating_menu, &Gen_conf_menu, NULL};

 
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
  //myPID.SetTunings(cfg.Kp, cfg.Ki, cfg.Kd);
  //myPID.Setpoint(cfg.SetPoint);
}

void temp_status(void* val){
  char t_str[6];
  static int second_view = 10;
  bool second_view_b;

  second_view_b = (second_view < 5);
  
  dtostrf(tempC, 2, 2, t_str);
  
  if (second_view_b){
    sprintf( (char*) val, "T: %s M: %s", t_str, cooling ? "ON": "OFF");
    dtostrf(cfg.cooling_sp, 2, 2, t_str);
    sprintf((char*) val+16, "CS: %s F: %d",t_str, fan);
  }else{
    sprintf( (char*) val, "T: %s H: %s", t_str, heating ? "ON": "OFF");
    dtostrf(cfg.heating_sp, 2, 2, t_str);
    sprintf((char*) val+16, "HS: %s F: %d",t_str, fan);
  }
    
  if (second_view-- == 0)
    second_view = 10;
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
  

}

 
void loop(){
  buttons.handleButtons();
  

  unsigned long curtime = millis();
  if( ((curtime - lastDspUpdte) > 1000)){
    lastDspUpdte = curtime;
    tempC = sensors.getTempC(sensor1);
    sensors.requestTemperatures(); 
    menu_handler.Render();
    
    
    if (!cooling && (tempC >= (cfg.cooling_sp + cfg.cooling_hyst)))
      cooling = true;

    if (cooling && (tempC <= cfg.cooling_sp))
      cooling = false;

    cooling = cooling && cfg.cooling_en;

    if (!heating && (tempC <= (cfg.heating_sp - cfg.heating_hyst)))
      heating = true;

    if (heating && (tempC >= cfg.heating_sp))
      heating = false;

    heating = heating && cfg.heating_en;
    
    if (cooling && (cfg.cooling_f_speed > 0)){
      fan = cfg.cooling_f_speed;
    }else if (heating && (cfg.heating_f_speed > 0)){
      fan = cfg.heating_f_speed;
    }else{
      fan = 0;
    }
    
    analogWrite(3, fan);
    digitalWrite(PIN_A1, cooling);
    //digitalWrite(PIN_A1, heating);
    analogWrite(11, cfg.contrast);
  }  
}
