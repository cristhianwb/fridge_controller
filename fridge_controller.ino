 
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>
#include <PID_v2.h>
#include <EEPROM.h>
#include "menu.h"

#include "button.h"
#include <avr/pgmspace.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define BT_DOWN 8
#define BT_UP 9
#define BT_LEFT 10
#define BT_RIGHT 12


#define NUM_STR_SZ 5
#define NUM_SPR_POS 2

#define OPTION_COUNT 7


const char opt_0[] PROGMEM = "Temp: "; // "String 0" etc são as strings a serem armazenadas - adapte ao seu programa.
const char opt_1[] PROGMEM = "SetPo.: ";
const char opt_2[] PROGMEM = "Hister.: ";
const char opt_3[] PROGMEM = "Contrst.";
const char opt_4[] PROGMEM = "PID (P):";
const char opt_5[] PROGMEM = "PID (I):";
const char opt_6[] PROGMEM = "PID (D):";

const char *const options_str[] PROGMEM = {opt_0, opt_1, opt_2, opt_3, opt_4, opt_5, opt_6};


 
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(7, 6, 5, 4, A5, 2);
uint8_t btns[] = {BT_DOWN, BT_UP, BT_LEFT, BT_RIGHT};
Button buttons(btns, 4, 50);


#define ONE_WIRE_BUS 14

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

float tempC = 0;

unsigned long lastDspUpdte = 0;
int ledState = LOW;
int pinpressed;

//MENU DEFS
uint8_t cur_line = 0;
uint8_t cur_col = 0;
uint8_t field_pos = 0;

char values[OPTION_COUNT][6] = {"01.00", "02.00", "03.00", "30  ","00.00", "00.00", "00.00"};
MENU_TYPE menu_types[OPTION_COUNT] = {MN_FLOAT, MN_FLOAT, MN_FLOAT, MN_INT, MN_FLOAT, MN_FLOAT, MN_FLOAT};


double Kp = 2, Ki = 5, Kd = 1;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

void print_status(){
  char status_str[16];
  char t_str[10];
  lcd.noCursor();
  dtostrf(tempC, 2, 2, t_str);
  sprintf(status_str, "Temp.: %s", t_str);
  lcd.print(status_str);
  lcd.setCursor(0, 1);
  sprintf(status_str, "%d %d", cur_line, cur_col);
  lcd.print(status_str);
}


void callback(uint8_t btPin){
    char tmp_str[10];
    char menu_str[16];
    int tmp;
  

    lcd.clear();
    lcd.setCursor(0, 0);

    switch(btPin){
      case BT_UP:
        if (cur_col == 0){
          if (cur_line > 0)
            cur_line--;
        } else{
          if (menu_types[cur_line-1] == MN_FLOAT){
            uint8_t val_col = cur_col - field_pos;
            if (values[cur_line-1][val_col] < '9')
              values[cur_line-1][val_col]++;
          }else if(menu_types[cur_line-1] == MN_INT){
              tmp = atoi(values[cur_line-1]);
              //limite superior ?
              //if ()
              tmp++;
              sprintf(values[cur_line-1], "%d", tmp);                
              
          }            
        }
        
      break;

      case BT_DOWN:
        
        if (cur_col == 0){
          if (cur_line < OPTION_COUNT)
            cur_line++;
        }else{
          if (menu_types[cur_line-1] == MN_FLOAT){
            uint8_t val_col = cur_col - field_pos;
            if (values[cur_line-1][val_col] > '0')
              values[cur_line-1][val_col]--;              
          }else if(menu_types[cur_line-1] == MN_INT){
            tmp = atoi(values[cur_line-1]);
            //limite inferior? por enquanto 0
            if (tmp > 0) tmp--;
            sprintf(values[cur_line-1], "%d", tmp);                
          }       
        }
        
      break;

      case BT_RIGHT:
        if (cur_line == 0)
          break;
        if (cur_col == 0){
          cur_col = field_pos;
        } else if (cur_col == field_pos + NUM_SPR_POS - 1){
          cur_col += 2;
        } else if (cur_col < (field_pos + NUM_STR_SZ - 1)){
          cur_col++;
        }
      break;

      case BT_LEFT:
        if (cur_line == 0)
          break;
        if (cur_col == field_pos){
          cur_col = 0;
        }else if (cur_col == field_pos + NUM_SPR_POS + 1){
          cur_col -= 2;
        }else if (cur_col > 0)
          cur_col--;  
      break;
      
      
    }
    // in the first line the menu is the status menu
    if (cur_line == 0){
      return;
    }

    
    // the other lines are the parameters menu
    strcpy_P(tmp_str, (char *) pgm_read_word(&(options_str[cur_line-1])));
    field_pos = sprintf(menu_str, "%d %s", cur_line, tmp_str);

    int lamp_val((int) (10.0 * atof(values[0])));
    analogWrite(3,  lamp_val);
    
    lcd.print(menu_str);
    lcd.print(values[cur_line-1]);
    lcd.setCursor(0, 1);
    char lval_str[8];
    tmp = atoi(values[3]);
    sprintf(lval_str, "%d", tmp);
    lcd.print(lval_str);
    lcd.cursor();
    lcd.setCursor(cur_col, 0);
    
    analogWrite(11, tmp);
    
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
  
  unsigned long curtime = millis();
  if( ((curtime - lastDspUpdte) > 1000 && (cur_line == 0) )){
    lastDspUpdte = curtime;
    sensors.requestTemperatures();
    tempC = sensors.getTempC(sensor1); 
    lcd.clear();
    lcd.setCursor(0, 0);
    print_status();
  }
  
}
