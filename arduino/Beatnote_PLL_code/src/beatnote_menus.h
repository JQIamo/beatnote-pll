#ifndef BEATNOTE_MENUS_H
#define BEATNOTE_MENUS_H

#include "Menu.h"
#include "LCD.h"
#include "AD9910.h"
#include "ADF4107.h"
#include "CSRManager.h"

extern LCD_ST7032 lcd;
extern AD9910 dds;
extern ADF4107 pll;

// Names in enum must match names used for CSR in .ino file
enum { P, B, A, R, POL, DDS_FREQ, REG_COUNT};
CSRManager csr(REG_COUNT);

extern uint16_t pVal;
extern uint16_t bVal;
extern uint16_t aVal;
extern uint16_t rVal;
extern uint16_t polVal;
extern uint32_t ddsFreq;
extern const char * gitversion;

// #ifndef GITVERSION
//   #define GITVERSION "UNKNOWN"
// #endif

/******** P Menu ************/

void update_p(int & num){
  // set P to 8*2^num (i.e. 8, 16, 32, 64)
  pVal = (uint16_t) (8 << num);
}

/******** P Menu ************/

void update_pol(int & num){
  // set P to 8*2^num (i.e. 8, 16, 32, 64)
  polVal = (uint16_t) num;
}

/******** B Menu ************/

void enter_b(uint16_t & num){
  // set initial display to correct value
  num = bVal;
}

void update_b(uint16_t & num){
  // constrain B in allowed range
  num = constrain(num, 3, 8191);
  // update B
  bVal = num;
}

/******** A Menu ************/

void enter_a(uint16_t & num){
  // set initial display to correct value
  num = aVal;
}

void update_a(uint16_t & num){
  // constrain A in allowed range
  num = constrain(num, 0, 63);
  // update A
  aVal = num;
}

/******** R Menu ************/

void enter_r(uint16_t & num){
  // set initial display to correct value
  num = rVal;
}

void update_r(uint16_t & num){
  // constrain A in allowed range
  num = constrain(num, 1, 16383);
  // update A
  rVal = num;
}

/******** DDS Menu ************/

void enter_dds(double & freq){
  // freq in MHz, dds operates in Hz
  freq = (double)ddsFreq/1.0e6;
}

void update_dds(double & freq){

  freq = constrain(freq, 0.0, 400.0);
  // freq in MHz, dds operates in Hz
  ddsFreq = (uint32_t)(freq*1.0e6);

  dds.setFreq(ddsFreq);
  // this could likely be bundled with above line;
  // leaving for now to double check float <-> int conversions
  // aren't dropping significant digits.
  freq = (double)ddsFreq/1.0e6;
}

void reset_dds(int & v){
  // this should be refactored
  dds.begin(40000000, 25);
  dds.setFreq(ddsFreq);
}

void code_version(int & v){

}

void update_pll(int & v){
  // push P, B, A, R, pol to the PLL
  pll.update(pVal, bVal, aVal, rVal, polVal);
}
/******** Menu Def ************/

uint16_t b_incr[4] = {1000, 100, 10, 1};
uint8_t b_cursor[4] = {0, 1, 2, 3};

uint16_t a_incr[2] = {10, 1};
uint8_t a_cursor[2] = {0, 1};

uint16_t r_incr[4] = {1000, 100, 10, 1};
uint8_t r_cursor[4] = {1, 2, 3, 4};

double freq_incr[6] = {10.0, 1.0, 0.1, 0.01, 0.001, 0.0001};
uint8_t freq_cursor[6] = {1, 2, 4, 5, 6, 7};

MenuNumeric<uint16_t> b_menu = MenuNumeric<uint16_t>::create("SET B           ", &lcd)
    .line0("B:              ")
    .line1("                ")
    .incr(b_incr, 4, 3) // Default to smallest increment
    .cursors(b_cursor)
    .formatter("%04.4i", 0, 1)
    .update_cb(update_b)
    .enter_cb(enter_b);

MenuNumeric<uint16_t> a_menu = MenuNumeric<uint16_t>::create("SET A           ", &lcd)
    .line0("A:              ")
    .line1("                ")
    .incr(a_incr, 2, 1)
    .cursors(a_cursor)
    .formatter("%02.2i", 0, 1)
    .update_cb(update_a)
    .enter_cb(enter_a);

MenuNumeric<uint16_t> r_menu = MenuNumeric<uint16_t>::create("SET R           ", &lcd)
    .line0("R:              ")
    .line1("                ")
    .incr(r_incr, 4, 3)
    .cursors(r_cursor)
    .formatter("%05.5i", 0, 1)
    .update_cb(update_r)
    .enter_cb(enter_r);

MenuNumeric<double> dds_menu = MenuNumeric<double>::create("DDS freq.       ", &lcd)
    .line0("dds:            ")
    .line1("             MHz")
    .incr(freq_incr, 6, 1)
    .cursors(freq_cursor)
    .formatter("%08.4f", 0, 1)
    .update_cb(update_dds)
    .enter_cb(enter_dds);

// Menu items to allow selection of P
MenuSelection p_8("08              ", &lcd, update_p, 0);
MenuSelection p_16("16              ", &lcd, update_p, 1);
MenuSelection p_32("32              ", &lcd, update_p, 2);
MenuSelection p_64("64              ", &lcd, update_p, 3);
// Menu items to allow selection of pol
MenuSelection pol_minus("-               ", &lcd, update_pol, 0);
MenuSelection pol_plus("+               ", &lcd, update_pol, 1);
// Menu item to program the PLL
MenuSelection prog_pll("Program PLL     ", &lcd, update_pll, 0);
// selection to reset the DDS
MenuSelection reset_dds_menu("Reset DDS       ", &lcd, reset_dds);
// selection to show code version
MenuSelection code_version_menu(gitversion, &lcd, code_version);
// should make a string menu
MenuController code_vers("Code vers >     ", &lcd);
// Menu to hold P values
MenuController p_menu("SET P >         ", &lcd);
// Menu to hold pol values
MenuController pol_menu("SET Pol. >      ", &lcd);
// Menu to hold reset_dds and code_version
MenuController misc("Misc >          ", &lcd);
// Main menu
MenuController root("Menu >          ", &lcd);

#endif
