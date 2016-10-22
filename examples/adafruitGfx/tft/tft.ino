#include <Arduino.h>

/********************
Sept. 2014 ~ Oct 2016 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com
creative commons license 3.0: Attribution-ShareAlike CC BY-SA
This software is furnished "as is", without technical support, and with no
warranty, express or implied, as to its usefulness for any purpose.

Thread Safe: No
Extensible: Yes

menu with adafruit GFX
output: Nokia 5110 display (PDC8544 HW SPI)
input: Serial
www.r-site.net
***/

#include <SPI.h>
#include <menu.h>
#include <dev/encoderIn.h>//for PCINT encoder
#include <dev/keyIn.h>//for encoder button
#include <dev/chainStream.h>//for mixing input stream (encoder+button)
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <dev/adafruitGfxOut.h>
#include <dev/serialOut.h>
//#include <FreeMono9pt7b.h>

using namespace Menu;

//ST7735 1.8TFT 128x160
#define TFT_CS   A1
#define TFT_DC   A0
#define TFT_RST  A2

Adafruit_ST7735 gfx(TFT_CS, TFT_DC, TFT_RST);

#define LEDPIN A4

// rotary encoder pins
#define encA    2
#define encB    3
#define encBtn  4

result zZz() {Serial<<"zZz"<<endl;return proceed;}

result showEvent(eventMask e,navNode& nav,prompt& item) {
  Serial<<e<<" on "<<item<<endl;
  return proceed;
}

int test=55;

result action1(eventMask e) {
  Serial<<e<<" action1 executed, proceed menu"<<endl;Serial.flush();
  return proceed;
}

result action2(eventMask e, navNode& nav, prompt &item, Stream &in, menuOut &out) {
  Serial<<item<<" "<<e<<" action2 executed, quiting menu"<<endl;
  return quit;
}

int ledCtrl=LOW;

result ledOn() {
  ledCtrl=HIGH;
  return proceed;
}
result ledOff() {
  ledCtrl=LOW;
  return proceed;
}

TOGGLE(ledCtrl,setLed,"Led: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);

int selTest=0;
SELECT(selTest,selMenu,"Select",doNothing,noEvent,noStyle
  ,VALUE("Zero",0,doNothing,noEvent)
  ,VALUE("One",1,doNothing,noEvent)
  ,VALUE("Two",2,doNothing,noEvent)
);

int chooseTest=-1;
CHOOSE(chooseTest,chooseMenu,"Choose",doNothing,noEvent,noStyle
  ,VALUE("First",1,doNothing,noEvent)
  ,VALUE("Second",2,doNothing,noEvent)
  ,VALUE("Third",3,doNothing,noEvent)
  ,VALUE("Last",-1,doNothing,noEvent)
);

//customizing a prompt look!
//by extending the prompt class
class altPrompt:public prompt {
public:
  altPrompt(const promptShadow& p):prompt(p) {}
  void printTo(idx_t i,navNode &nav,menuOut& out) override {
    out<<"special prompt!";
  }
};

MENU(subMenu,"Sub-Menu",showEvent,anyEvent,noStyle
  ,OP("Sub1",showEvent,anyEvent)
  ,OP("Sub2",showEvent,anyEvent)
  ,OP("Sub3",showEvent,anyEvent)
  ,altOP(altPrompt,"",showEvent,anyEvent)
  ,EXIT("<Back")
);

result alert(menuOut& o,idleEvent e) {
  if (e==idling)
    o<<"alert test"<<endl<<"press [select]"<<endl<<"to continue..."<<endl;
  return proceed;
}

result doAlert(eventMask e, navNode& nav, prompt &item, Stream &in, menuOut &out) {
  nav.root->idleOn(alert);
  return proceed;
}

MENU(mainMenu,"Main menu",zZz,noEvent,wrapStyle
  ,OP("Op1",action1,anyEvent)
  ,OP("Op2",action2,enterEvent)
  ,FIELD(test,"Test","%",0,100,10,1,doNothing,noEvent)
  ,SUBMENU(subMenu)
  ,SUBMENU(setLed)
  ,OP("LED On",ledOn,enterEvent)
  ,OP("LED Off",ledOff,enterEvent)
  ,SUBMENU(selMenu)
  ,SUBMENU(chooseMenu)
  ,OP("Alert test",doAlert,enterEvent)
  ,EXIT("<Back")
);

// define menu colors --------------------------------------------------------
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
//monochromatic color table
const colorDef<uint16_t> colors[] MEMMODE={
  {{BLACK,BLACK},{BLACK,BLUE,BLUE}},//bgColor
  {{GRAY,GRAY},{WHITE,WHITE,WHITE}},//fgColor
  {{WHITE,BLACK},{YELLOW,YELLOW,RED}},//valColor
  {{WHITE,BLACK},{WHITE,YELLOW,YELLOW}},//unitColor
  {{WHITE,GRAY},{BLACK,BLUE,WHITE}},//cursorColor
  {{WHITE,YELLOW},{BLUE,RED,RED}},//titleColor
};

encoderIn encoder(encA,encB);//simple quad encoder driver
encoderInStream encStream(encoder,4);// simple quad encoder fake Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,options.getCmdChar(enterCmd)}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys

//input from the encoder + encoder button + serial
Stream* inputsList[]={&encStream,&encButton,&Serial};
chainStream<3> in(inputsList);//3 is the number of inputs

#define textScale 1
//font size is 6x9
//const panel panels[] MEMMODE={{0,0,160/6/textScale,128/9/textScale}};
//testing smaller panels and positioning
const panel panels[] MEMMODE={{0,0,18,8}};
panelsList pList(panels,1);

serialOut outSerial(Serial,pList);//the output device (just the serial port)
//font size is 6x9
menuGFX outGFX(gfx,colors,pList,6*textScale,9*textScale);//output device for LCD
menuOut* outputs[]={&outGFX,&outSerial};
outputsList out(outputs,1);
NAVROOT(nav,mainMenu,2,in,out);

//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  if (e==idling)
    o<<"suspended..."<<endl<<"press [select]"<<endl<<"to continue"<<endl<<(millis()%1000);
  return proceed;
}

void setup() {
  pinMode(LEDPIN,OUTPUT);
  Serial.begin(115200);
  while(!Serial);
  Serial<<"menu 3.0 test"<<endl;Serial.flush();
  options.idleTask=idle;//point a function to be used when menu is suspended
  mainMenu[1].enabled=disabledStatus;
  nav.showTitle=false;

  pinMode(encBtn, INPUT_PULLUP);
  encoder.begin();

  SPI.begin();
  gfx.initR(INITR_BLACKTAB);
  gfx.setRotation(3);
  gfx.setTextSize(textScale);//test scalling
  gfx.setTextWrap(false);
  gfx.fillScreen(BLACK);
  gfx.setTextColor(ST7735_RED,ST7735_BLACK);
  gfx<<"Menu 3.x test on GFX"<<endl;;
  delay(2000);
}

void loop() {
  nav.poll();
  digitalWrite(LEDPIN, ledCtrl);
  delay(100);//simulate a delay when other tasks are done
}