#include <U8g2lib.h>
#include <Wire.h>

//main include for ArduinoMenu
#include <menu.h>
//input/output drivers --------------------------------------
#include <menu/IO/Arduino/serialIn.h>
#include <menu/IO/Arduino/serialOut.h>
//format specifyers -----------------------------------------
#include <menu/fmt/text.h>
#include <menu/fmt/titleWrap.h>
//components ------------------------------------------------
#include <menu/comp/endis.h>
#include <menu/comp/numField.h>

//wemos lolin 32 + oled + u8g2
#define SDA 5
#define SCL 4
#define fontName u8g2_font_7x13_mf

U8G2_SSD1306_128X64_VCOMH0_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL, SDA);//allow contrast change

// some user code for example --------------------------------------------------------------
bool running=true;
int year=1967;
int vcc=3;

//handler for "Quit" option
bool quit() {
  //just signal program exit
  trace(MDO<<"Quit!"<<endl);
  running=false;
  return true;
}

// define menu structure ---------------------------------------------------------------

//a single rogue test, not part of a menu
Text<> test("Just a test");

//texts for menu
const char* mainMenu_title="Main menu";
const char* subMenu_title="Sub-menu";
const char* op1_text="Option 1";
const char* op2_text="Option 2";
const char* opn_text="Option ...";
const char* exit_txt="<Exit";
const char* quit_txt="<Quit";
const char* yr_txt="Year";
const char* vcc_txt="VCC";
const char* volts_txt="V";

//static menu structure
StaticMenu<
  Item<StaticText<&mainMenu_title>>,
  StaticData<
    Item<EnDis<StaticText<&op1_text>>>,
    Item<EnDis<StaticText<&op2_text>>>,
    Item<NumField<StaticText<&yr_txt>,int,year,1900,2100,10,1>>,//this is NOT good, changing limits generates new code->TODO: add a translation
    Item<NumField<StaticText<&vcc_txt>,decltype(vcc),vcc,0,100,1,0,StaticText<&volts_txt>>>,
    Item<StaticText<&opn_text>>,
    StaticMenu<
      Item<StaticText<&subMenu_title>>,
      StaticData<
        Item<EnDis<StaticText<&op1_text>>>,
        Item<StaticText<&op2_text>>,
        Item<StaticText<&opn_text>>,
        Item<StaticText<&opn_text>>,
        Item<StaticText<&opn_text>>,
        Item<Exit<StaticText<&exit_txt>>>
      >
    >,
    Item<Action<StaticText<&quit_txt>,quit>>
  >
> mainMenu;//create menu object

//menu input --------------------------------------
SerialIn<decltype(Serial),Serial> in;//create input object (here serial)

//menu output (Serial)
using Out=FullPrinter<//print title and items
  Fmt<//formating API
    TitleWrapFmt<//put [] around menu title
      TextFmt<//apply text formating
        RangePanel<//scroll content on output geometry
          StaticPanel<0,0,20,4,SerialOut<decltype(Serial),Serial>>//describe output geometry and device
        >
      >
    >
  >
>;

Out out;//create output object (Serial)

//navigation root ---------------------------------
Nav<decltype(mainMenu),mainMenu,2> nav;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Wire.begin(SDA,SCL);
  u8g2.setFont(fontName);
  u8g2.begin();
  u8g2.setContrast(255);
  do {
    u8g2.drawStr(0,63,"www.r-site.net");
  } while ( u8g2.nextPage() );
  delay(1000);
  // test.print(out);//printing single field
  // Console::nl();
  nav.printMenu(out);
}

void loop() {
  if (nav.doInput(in)) nav.printMenu(out);
}