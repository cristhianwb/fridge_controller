 
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>

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

#define OPTION_COUNT 4


const char opt_0[] PROGMEM = "Temp: "; // "String 0" etc são as strings a serem armazenadas - adapte ao seu programa.
const char opt_1[] PROGMEM = "SetPo.: ";
const char opt_2[] PROGMEM = "Hister.: ";
const char opt_3[] PROGMEM = "Contrst.";

const char *const options_str[] PROGMEM = {opt_0, opt_1, opt_2, opt_3};


 
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(7, 6, 5, 4, A5, 2);
uint8_t btns[] = {8,9,10,12};
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

char * values[4] = {"01.00", "02.00", "03.00", "0"};


void print_status(){
  char status_str[16];
  char t_str[10];
  lcd.noCursor();
  dtostrf(tempC, 2, 2, t_str);
  sprintf(status_str, "Temp.: %s", t_str);
  lcd.print(status_str);
}


void callback(uint8_t btPin){
    char tmp_str[10];
    char menu_str[16];
  

    lcd.clear();
    lcd.setCursor(0, 0);

    switch(btPin){
      case BT_DOWN:
        if ((cur_col == 0) && (cur_line > 0)){
          cur_line--;
        } else {
          uint8_t val_col = cur_col - field_pos;
          if (values[cur_line-1][val_col] > '0')
            values[cur_line-1][val_col]--;          
        }
      break;

      case BT_UP:
        if ((cur_col == 0) && (cur_line < OPTION_COUNT)){
          cur_line++;
        }else{
          uint8_t val_col = cur_col - field_pos;
          if (values[cur_line-1][val_col] < '9')
            values[cur_line-1][val_col]++;       
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
      print_status();
      return;
    }

    
    // in the rest of the lines is the parameters menu
    strcpy_P(tmp_str, (char *) pgm_read_word(&(options_str[cur_line-1])));
    field_pos = sprintf(menu_str, "%d %s", cur_line, tmp_str);

    int lamp_val((int) (10.0 * atof(values[0])));
    analogWrite(3,  lamp_val);
    
    lcd.print(menu_str);
    lcd.print(values[cur_line-1]);
    lcd.setCursor(0, 1);
    char lval_str[8];
    sprintf(lval_str, "%d", lamp_val);
    lcd.print(lval_str);
    lcd.cursor();
    lcd.setCursor(cur_col, 0);
    
}
 
void setup()
{
  //pinMode(A7, OUTPUT);
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  lcd.cursor();

  //Define o constraste
  pinMode(11, OUTPUT);
  analogWrite(3, 30);

  //Setup of the debounce algo
  //pinMode(buttonPin, INPUT_PULLUP);
  //pinMode(ledPin, OUTPUT);
  buttons.Setup();
  buttons.setCallback(callback);
  sensors.getAddress(sensor1, 0);
  pinMode(3, OUTPUT);
     
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
