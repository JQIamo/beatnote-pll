/*
   BeatnoteBoxPro.ino  - Arduino sketch for Beatnote Box; final "pro" version.
   
   ie, for use with Teensy_FrequencyShield_slim.h
      
   Created by Neal Pisenti, 2015
   JQI - Joint Quantum Institute

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// For use with a Teensy; may work with other arduino-style boards too.

#include "Teensy_FrequencyShield_2ch_slim.h"

#include <SPI.h>

#include "Encoder.h"
#include "LCD.h"
#include "Menu.h"
#include "EEPROM.h"

#include "ADF4107.h"

ADF4107 beatnotePLL(PLL1_LE);

// declare LCD
LCD lcd(LCD_RST, LCD_RS, LCD_CSB);


//Initialize Encoder Objects
Encoder set_p(ENC_A2, ENC_B2, ENC_SW2);
Encoder set_b(ENC_A2, ENC_B2, ENC_SW2);
Encoder set_a(ENC_A2, ENC_B2, ENC_SW2);
Encoder set_r(ENC_A2, ENC_B2, ENC_SW2);
Encoder set_pol(ENC_A2, ENC_B2, ENC_SW2);

Encoder change_menu(ENC_A1, ENC_B1, ENC_SW1);

Encoder menu_context_sequence[5] = { set_p, set_b, set_a, set_r, set_pol };


enum MENUS { MENU_MAIN, MENU_P, MENU_B, MENU_A, MENU_R, MENU_POL, MENU_COUNT };
int currentMenu = MENU_MAIN;
int lastMenu = MENU_MAIN;
volatile bool updateLCD = true;

double currentMultiplier = 0.0;

// map to p values
double p_map[4] = {8.0, 16.0, 32.0, 64.0};

Menu menu(MENU_COUNT);

#define INIT_P 0.0
#define INIT_B 3.0
#define INIT_A 1.0
#define INIT_R 1.0
#define INIT_POL 5000.0


void dummy_event(Encoder *this_encoder) {
  //Serial.println("here");
}


void push_update(Encoder *encoder){
    lcd.write("Writing new...  ", 0x40);
  //  int pol = (int)set_pol.po   
    beatnotePLL.initialize((int)p_map[(int)set_p.position()], (int)set_b.position(), (int)set_a.position(), (int)set_r.position(), ((int)set_pol.position() % 2), 0);
    delay(800);
    currentMultiplier = calcMultiplier();

  // update EEPROM
  float p = set_p.position();
  Serial.println(p);
  Serial.println(set_p.position());
  float b = set_b.position();
  float a = set_a.position();
  float r = set_r.position();
  float pol = set_pol.position();
  
  int addr = sizeof(int);
        addr = sizeof(int);
      EEPROM.put(addr, p);
      addr += sizeof(int);
      EEPROM.put(addr, b);
      addr += sizeof(int);
      EEPROM.put(addr, a);
      addr += sizeof(int);
      EEPROM.put(addr, r);
      addr += sizeof(int);
      EEPROM.put(addr, pol);

}


void interruptWrapper() {
 if (menu.current_mode() == MENU_P) {
    set_p.interrupt();
  } else if (menu.current_mode() == MENU_B) {
    set_b.interrupt();
  } else if (menu.current_mode() == MENU_A) {
    set_a.interrupt();
  } else if (menu.current_mode() == MENU_R) {
    set_r.interrupt();
  } else if (menu.current_mode() == MENU_POL) {
    set_pol.interrupt();
  }
  updateLCD = true;
 //Serial.println("here");
}

void menuInterruptWrapper(){
    change_menu.interrupt();
    updateLCD = true;
    //Serial.println("over");
}


void menu_main() {


        char buff[17];
        snprintf(buff, 17, "Mult: %-16.4f", currentMultiplier);
        lcd.write(buff, 0x00);
        
        double newMult = calcMultiplier();
        if (int(100*newMult) != int(100*currentMultiplier)){
            snprintf(buff, 17, "New: %-16.4f", calcMultiplier());
            lcd.write(buff, 0x40);
        } else {
            lcd.write("Mult = (P*B+A)/R", 0x40);
        }
        
         change_menu.button_events();
}

double calcMultiplier(){
    double p = p_map[(int)set_p.position()];
    double b = set_b.position();
    double a = set_a.position();
    double r = set_r.position();
    return (p*b+a)/r;
}

void menu_p() {
    
        char buff[17];
       
        snprintf(buff, 17, "P: %-16i", (int)p_map[(int)set_p.position()]);
        lcd.write(buff, 0x00);
        snprintf(buff, 17, "Mult: %-16.4f", calcMultiplier());
        lcd.write(buff, 0x40);


}

void menu_b() {
 
        char buff[17];
       
        snprintf(buff, 17, "B: %-16i", (int)set_b.position());
        lcd.write(buff, 0x00);
        snprintf(buff, 17, "Mult: %-16.4f", calcMultiplier());
        lcd.write(buff, 0x40);

}

void menu_a() {
         char buff[17];
       
        snprintf(buff, 17, "A: %-16i", (int)set_a.position());
        lcd.write(buff, 0x00);
        snprintf(buff, 17, "Mult: %-16.4f", calcMultiplier());
        lcd.write(buff, 0x40);
        
}

void menu_r() {

               char buff[17];
       
        snprintf(buff, 17, "R: %-16i", (int)set_r.position());
        lcd.write(buff, 0x00);
        snprintf(buff, 17, "Mult: %-16.4f", calcMultiplier());
        lcd.write(buff, 0x40);
      
}

void menu_pol() {
 
        char buff[17];
       
        snprintf(buff, 17, "POL: %-16i", ((int)set_pol.position() % 2));
        lcd.write(buff, 0x00);
        snprintf(buff, 17, "Mult: %-16.4f", calcMultiplier());
        lcd.write(buff, 0x40);
       
}




void setup(){

    Serial.begin(9600);
    delay(500);

  //Set up encoder pins:
  pinMode(ENC_A1, INPUT_PULLUP);
  pinMode(ENC_B1, INPUT_PULLUP);
  pinMode(ENC_SW1, INPUT_PULLUP);
  pinMode(ENC_A2, INPUT_PULLUP);
  pinMode(ENC_B2, INPUT_PULLUP);
  pinMode(ENC_SW2, INPUT_PULLUP); 

  //Initialize the LCD
  SPI.begin();
  delay(100);
  lcd.init();
  
      //Initialize encoders
    double step_sizes[1] = {1.0};
    String step_labels[1] = {"1"};


    // load from eeprom;
    float init_p;
    float init_b;
    float init_a;
    float init_r;
    float init_pol;
    
    int addr = 0;
    int special = EEPROM.read(addr);
    Serial.print("reading special ");
    Serial.println(special);
    Serial.print("thing: ");
    Serial.println(0xBE);
    if (special != 0xBE){
      Serial.println("inits");
      // initialize EEPROM
      EEPROM.put(addr, 0xBE);
      addr = sizeof(int);
      EEPROM.put(addr, INIT_P);
      addr += sizeof(int);
      EEPROM.put(addr, INIT_B);
      addr += sizeof(int);
      EEPROM.put(addr, INIT_A);
      addr += sizeof(int);
      EEPROM.put(addr, INIT_R);
      addr += sizeof(int);
      EEPROM.put(addr, INIT_POL);
      
    }

    // read EEPROM values
      addr = sizeof(int);
        EEPROM.get(addr, init_p);
        addr += sizeof(float);
        EEPROM.get(addr, init_b);
        addr += sizeof(float);
        EEPROM.get(addr, init_a);
         addr += sizeof(float);
        EEPROM.get(addr, init_r);
          addr += sizeof(float);
        EEPROM.get(addr, init_pol);
   
    set_p.init(init_p, 0.0, 3.0);
    set_b.init(init_b, 3.0, 8191.0);
    set_a.init(init_a, 0.0, 63.0); 
    set_r.init(init_r, 1.0, 16383.0);
    set_pol.init(init_pol, 0.0, 10000.0);
    
    for (int i = 0; i < 5; i++){
        menu_context_sequence[i].define_step_sizes(1, step_sizes, step_labels);
        menu_context_sequence[i].attach_button_press_event(dummy_event);
        menu_context_sequence[i].attach_button_hold_event(dummy_event);
    }
    
    // cheeky way of using mod arithmetic
    change_menu.init(6000.0, 0.0, 10000.0);
    change_menu.define_step_sizes(1, step_sizes, step_labels);
    change_menu.attach_button_press_event(dummy_event);
    change_menu.attach_button_hold_event(push_update);
    
    // attach interrupts
    attachInterrupt(ENC_A1, menuInterruptWrapper, CHANGE);
    attachInterrupt(ENC_A2, interruptWrapper, CHANGE);

    menu.attach_mode(MENU_MAIN, "Main Menu", menu_main);
    menu.attach_mode(MENU_P, "P Menu", menu_p);
    menu.attach_mode(MENU_B, "B Menu", menu_b);
    menu.attach_mode(MENU_A, "A Menu", menu_a);
    menu.attach_mode(MENU_R, "R Menu", menu_r);
    menu.attach_mode(MENU_POL, "Pol Menu", menu_pol);
 
    
     currentMultiplier = calcMultiplier();
 
    menu.switch_to_mode(MENU_MAIN);
    menu.run_mode();
    //Serial.println(MENU_MAIN);
    
    beatnotePLL.initialize((int)p_map[(int)set_p.position()], (int)set_b.position(), (int)set_a.position(), (int)set_r.position(), ((int)set_pol.position() % 2), 0);
}

void loop(){


    currentMenu = int(change_menu.position()) % MENU_COUNT;
    menu.switch_to_mode(currentMenu); 
 
    menu.run_mode();
  
    

}
