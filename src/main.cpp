#include <Arduino.h>
#include "VentiData.h"
#include "Beat2020Panel.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>

#include "SerialManager.h"
#include "Beat2020WifiCom.h"

using namespace beat2020;
WifiCom<5> wifi;



// ####################### K E Y P A D   V A R I A B L E S ########################
const byte COLS = 4;            // 4 Columns
const byte ROWS = 3;            // 3 Rows
char hexaKeys[ROWS][COLS] = {     // Definition of certain char for every key
{'1','u','+','A'},
{'2','<','O','>'},
{'3','d','-','Z'}
};
byte colPins[COLS] = {6,7,8,9}; //Definition of 4 Column-Pins
byte rowPins[ROWS] = {3,4,5};   //Definition of 3 Row-Pins
char Button;                    // stores char of pressed key
boolean menuloop = false;



// ######################## D E C L A R E   O B J E C T S #########################
LiquidCrystal_I2C display(0x27, 20, 4);  // set the LCD address to 0x27 for a 20 chars and 4 line display
Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); // function for reading pressed key

Data setData;
Data actData;

SerialManager<Data, Data> manager(Serial1, actData, setData, FAST, FAST);

IParameter* setpoints[]{
	  new Parameter<uint8_t>("frequency f",setData.bpm = 15,"[bpm]",30,1),
    new Parameter<uint8_t>("press. Insp.",setData.p_ins = 20,"[mBar]",100,5),
	  new Parameter<uint8_t>("press. Exp.",setData.p_exp = 6,"[mBar]",20,1),
    new Parameter<uint8_t>("Dutycycle X",setData.duty = 50,"[%]",90,10)                      
};

IParameter* alarmpoints[]{
    new Parameter<uint8_t>("time tol. dt_rise",setData.dt_rise = 10,"[%]",50,1),
	  new Parameter<uint8_t>("time tol. dt_fall",setData.dt_fall = 10,"[%]",50,1),
	  new Parameter<uint8_t>("press. tol. dp_ins",setData.dp_ins = 10,"[%]",50,1),
    new Parameter<uint8_t>("press. tol. dp_exp",setData.dp_exp = 10,"[%]",50,1),
    new Parameter<uint8_t>("flow tol. dvol",setData.dvol = 10,"[%]",50,1)
};

IParameter* controlparams[]{
    new Parameter<uint16_t>("Proportional kp",setData.contr_kp = 100,"[1/mBar]",32768,1),
	  new Parameter<uint16_t>("Integral tn",setData.contr_tn = 500,"[ms]",32768,1),
	  new Parameter<uint16_t>("Differential tv",setData.contr_tv = 20,"[ms]",32768,1)
};

SetUpMenu menu1("   Setpoint-Menu    ", display, keypad, setpoints, 4);
SetUpMenu menu2("  Alarmpoint-Menu   ", display, keypad, alarmpoints, 5);
SetUpMenu menu3("   Control-Menu     ", display, keypad, controlparams, 3);
SetUpMenu menu4("   Control-Menu     ", display, keypad, controlparams, 3);

IMenu* submenus[4]{
  &menu1,
  &menu2,
  &menu3,
  &menu4
};

MainMenu menu(display, keypad, submenus, (byte)4);


void setup() {
  //Serial.begin(9600);
	Serial1.begin(9600);

  display.init();
  display.backlight();
  delay(1000);
  display.clear();
  display.setCursor(0,0);   display.print("      BEAT2020      ");
  display.setCursor(0,1);   display.print("    HTWK Leipzig    ");
  display.setCursor(0,2);   display.print("   Software V.1.0   ");
  manager.recieve();
  wifi.connect();
  delay(3000);
  manager.recieve();
  setData.bpm = actData.bpm;
  setData.p_ins = actData.p_ins;
  setData.p_exp = actData.p_exp;
  setData.duty = actData.duty;

  setData.dt_rise = actData.dt_rise;
  setData.dt_fall = actData.dt_fall;
  setData.dp_ins = actData.dp_ins;
  setData.dp_exp = actData.dp_exp;
  setData.dvol = actData.dvol;

  setData.contr_mode = actData.contr_mode;
  menu.state = setData.contr_mode;

  display.clear();
  display.setCursor(0,0);   display.print("f               bpm ");
  display.setCursor(0,1);   display.print("p_set           mBar");
  display.setCursor(0,2);   display.print("p_act           mBar");
  display.setCursor(0,3);   display.print("v_ins/exp           ");

  
}

void loop() {
  //Serial.println("Manager update");
  manager.update();
  //printPrameters
  //delay(100);
  display.setCursor(7,0);   display.print("         ");
	display.setCursor(7,1);   display.print("         ");
	display.setCursor(7,2);   display.print("         ");
  display.setCursor(10,3);  display.print("         ");

	display.setCursor(7,0);   display.print(actData.bpm);
	display.setCursor(7,1);   display.print(actData.p_set);
	display.setCursor(7,2);   display.print((((int16_t)actData.p_c)-2731)/218.44);

	if (actData.vol_exp == 0){
		display.setCursor(10,3);   display.print("inf");
	}else{
		display.setCursor(10,3);   display.print(actData.vol_insp*1.0/actData.vol_exp);
	}

  //Serial.println("Wifi update");
  wifi.addData(actData);
  wifi.update();

  //Serial.println("Keypad update");
  if('O'==keypad.getKey()){
      menu.update();
      display.setCursor(0,0);   display.print("f               Hz  ");
      display.setCursor(0,1);   display.print("p_set           mBar");
      display.setCursor(0,2);   display.print("p_act           mBar");
      display.setCursor(0,3);   display.print("v_ins/exp           ");
  }

  setData.contr_mode = menu.state;//0 = Off, 1 = ON
}
