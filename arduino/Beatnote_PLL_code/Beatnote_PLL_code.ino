#include <SPI.h>

#include "LCD.h"
#include "Menu.h"
#include "EncoderStream.h"
#include "CSRManager.h"
#include "Thread.h"

// pin definitions for TeensyBreakout.brd from beatnote_PLL repository 
// Note that some of these features are not yet implemented in library.
#define DDS_IOUPDATE  23
#define DDS_RESET     20
#define DDS_PS0       22
#define DDS_CS        21

#define AUX_TRIG      16  // input

#define LCD_CS        9
#define LCD_RS        10
#define LCD_RST       8

#define PLL_LE        14

#include "src/beatnote_menus.h"

// Line below requires Makefile (see aom-driver)
//const char * gitversion = GITVERSION;
// Hard code release version (make sure to update for new releases!)
const char * gitversion = "v2.0.0";

uint16_t pVal = 16;
uint16_t bVal = 3;
uint16_t aVal = 0;
uint16_t rVal = 1;
uint16_t polVal = 1;
uint32_t ddsFreq = 25000000;  // = 25MHz

LCD_ST7032 lcd(LCD_RST, LCD_RS, LCD_CS);
AD9910 dds(DDS_CS, DDS_RESET, DDS_IOUPDATE, DDS_PS0);
ADF4107 pll(PLL_LE);

using namespace EncoderStream;

int const EncoderStream::ENC_A = 7;
int const EncoderStream::ENC_B = 6;
int const EncoderStream::ENC_SW = 5;
//EncoderStream only supports one encoder.
//int const EncoderStream::ENC_A = 4;
//int const EncoderStream::ENC_B = 3;
//int const EncoderStream::ENC_SW = 2;

Thread saveThread = Thread();

void saveToEEPROM(){
  csr.save(DDS_FREQ);
  csr.save(P);
  csr.save(B);
  csr.save(A);
  csr.save(R);
  csr.save(POL);
}

//#define DEBUG // uncomment this line if you want serial debugging turned on.

void setup(){
  // set up save functions:
  csr.associate(DDS_FREQ, &ddsFreq);
  csr.associate(P, &pVal);
  csr.associate(B, &bVal);
  csr.associate(A, &aVal);
  csr.associate(R, &rVal);
  csr.associate(POL, &polVal);

  csr.init_from_eeprom();
  // start the save thread
  saveThread.onRun(saveToEEPROM);
  saveThread.setInterval(60000);  // save checkpoint every minute

  Serial.begin(9600);

#ifdef DEBUG
  while (!Serial){ ; }
  Serial.println("alive!!");
  delay(100);
#endif

  SPI.begin();
  delay(100);
  // initialize DDS
  dds.begin(40000000, 25);
  dds.setFreq(ddsFreq);
  // initialize PLL
  pll.initialize(pVal, bVal, aVal, rVal, polVal, 0);
  // initialize encoder stream
  Encoder.begin();
  // initialize LCD
  lcd.begin();
  lcd.cursor();
  // set up the PLL programming menu
  root.add(&prog_pll);
  root.add(&lock_menu);
  // set up prescaler menu
  // switch to ensure correct initial menu
  switch (pVal){
    case 8:
      p_menu.add(&p_8);
      p_menu.add(&p_16);
      p_menu.add(&p_32);
      p_menu.add(&p_64);
      break;
    case 16:
      p_menu.add(&p_16);
      p_menu.add(&p_32);
      p_menu.add(&p_64);
      p_menu.add(&p_8);
      break;
    case 32:
      p_menu.add(&p_32);
      p_menu.add(&p_64);
      p_menu.add(&p_8);
      p_menu.add(&p_16);
      break;
    case 64:
      p_menu.add(&p_64);
      p_menu.add(&p_8);
      p_menu.add(&p_16);
      p_menu.add(&p_32);
      break;
    default:
      p_menu.add(&p_8);
      p_menu.add(&p_16);
      p_menu.add(&p_32);
      p_menu.add(&p_64);
      break;
  }
  root.add(&p_menu);
  // set up b, a, r counter menus
  root.add(&b_menu);
  root.add(&a_menu);
  root.add(&r_menu);
  // set up polarity menu
  // switch to ensure correct initial menu
  switch (polVal){
    case 0:
      pol_menu.add(&pol_minus);
      pol_menu.add(&pol_plus);
      break;
    case 1:
      pol_menu.add(&pol_plus);
      pol_menu.add(&pol_minus);
      break;
    default:
      pol_menu.add(&pol_plus);
      pol_menu.add(&pol_minus);
      break;
  }
  root.add(&pol_menu);
  // set up dds menu
  // TODO: check if DDS installed by reading back from register
  // only enable menu if DDS available.
  root.add(&dds_menu);
  // set up misc menu
  misc.add(&reset_dds_menu);
  misc.add(&code_vers);
  code_vers.add(&code_version_menu);
  root.add(&misc);
  // enter the root menu
  root.enter();

}


void loop(){

  if (Encoder.available() > 0){
    char c = Encoder.read();
    root._active->process(c);
  }

  if (saveThread.shouldRun()){
    saveThread.run();
  }

  // SetListArduino could be added here as well too.

}
