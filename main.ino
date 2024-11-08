#include <MCUFRIEND_kbv.h>
#include <stdlib.h>
#include <SPI.h>
#include <SD.h>
#include <Fonts/FreeSans9pt7b.h>

MCUFRIEND_kbv tft;

#define BLACK   0x0000
#define GREEN   0x1402
#define MEH_GREEN 0x0a61
#define DARK_GREEN 0x0961

#define NAMEMATCH ".bmp"
#define PALETTEDEPTH   0     // do not support Palette modes
#define BMPIMAGEOFFSET 54
#define BUFFPIXEL      20

#define netXoffset 20
#define ARRAYSIZE 7
#define PADDING 4

#define sResX 480
#define sResY 320

int Upper_Tab = 1; //STAT, INV, DATA, MAP, RADIO
int Lower_Tab = 1;
//writes text to the screen
void showmsgXY(int x, int y, int sz, char msg[], uint16_t color, bool typewrite){
  //by default letters 6 pixels wide by 8 and then multiplied by the size
  tft.setFont( (sz == 0) ? NULL : &FreeSans9pt7b );
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(sz);

  if (typewrite){
    char *t;
    int size = 0;
   for (t = msg; *t != '\0'; t++) {
        tft.print(msg[size]);
        tone(A5, 1, 0.03);
        delay(20);
        noTone(A5);
        size++;
      }
  } else {
    tft.print(msg);
  }
}

//This will just create a visual on the pipboy if an error is present
void pipError(char errorMsg[] = "PLEASE CONTACT A ROBCO PERSONAL\nTHIS UNIT HAS MALFUNCTIONED\nIF A ROBCO PERSONAL IS UNAVAILABLE\nTHEN PRESS THE POWER BUTTON\nTO RESET THE PIPBOY"){
  // tft.drawRect(0,0,100,100, GREEN);
  showmsgXY(0, 15, 1, errorMsg, GREEN, true);
}

//ensures connection to screen, controls, sd, etc
//If this fails it will exit the program and let the user know
void initialize_connections(void)
{
  Serial.begin(9600);

  tft.begin(tft.readID());
  tft.invertDisplay(1);
  tft.setRotation(3);
  tft.fillScreen(BLACK);

  if (!SD.begin(10)){ pipError("THERE IS NO RAM OR STORAGE INSTALLED\n PLEASE INSERT AN SD CARD FOR FURTHER ANALYSIS"); }
}

class Pipboy{  
  public:
    int uppertab = 1;
    int lowertab = 1;
    void startup(void){  
      int spaceBetweenOSandSpecs = 40;
      float waitBeforeScroll = 1;
      float timeForRedraw = 0.1;
      char OS[] = "****************************** PIP_OS(R) V1.0.0.0 ******************************"; 
      char systemInfo[] = "COPYRIGHT 2075 ROBCO(R)\nLOADER V1.1\nEXEC VERSION 41.10\n32k RAM SYSTEM\n15508 BYTES FREE\nNO HOLOTAPE FOUND\nLOAD ROM(1): DEITRIX 303";
      showmsgXY(0,10,0,OS,GREEN,true);
      showmsgXY(0,10+spaceBetweenOSandSpecs,1,systemInfo,GREEN,true);

      delay(waitBeforeScroll * 1000);

      //Scrolls up the text (is slow on the uno but should be faster on esp 32)
      int prevY = 0;
      for (int i = 0; i < sResY-100; i+=10){
        //Rewrite text as black (speeds up screen redraw)
        showmsgXY(0,10-prevY,0,OS,BLACK,false);
        showmsgXY(0,10+spaceBetweenOSandSpecs-prevY,1,systemInfo,BLACK,false);
        showmsgXY(0,10-i,0,OS,GREEN,false);
        showmsgXY(0,10+spaceBetweenOSandSpecs-i,1,systemInfo,GREEN,false);
        prevY = i;
        delay(timeForRedraw);
      }
      tft.fillScreen(BLACK);

      //Intializing text
      int timesToLoad = 10;
      int timeBetweenDot = 100;
      int loadingOffsetX = 200;
      int loadingOffsetY = 250;
      //concatonate
      for (int i = 0; i < timesToLoad; i++){
        showmsgXY(loadingOffsetX, loadingOffsetY, 1, "Initializing.", GREEN, false);
        delay(timeBetweenDot);
        showmsgXY(loadingOffsetX, loadingOffsetY, 1, "Initializing..", GREEN, false);
        delay(timeBetweenDot);
        showmsgXY(loadingOffsetX, loadingOffsetY, 1, "Initializing...", GREEN, false);
        delay(timeBetweenDot);
        showmsgXY(loadingOffsetX, loadingOffsetY, 1, "Initializing...", BLACK, false);
      }
    };
    //STAT, INV, DATA, MAP, RADIO
    void loadStat(bool fullReset = true){
      //full reset is just if you wanna reset the whole page or just after changing the lower tab
      uppertab = 1;
      if (fullReset){
        tft.fillScreen(BLACK);
        tft.fillRect(0, 25, sResX, 2, GREEN);
        tft.fillRect(43, 7, 60, 19, GREEN);
        tft.fillRect(43 + PADDING/2, 7 + PADDING - 2, 60 - PADDING, 19, BLACK);
        tft.fillRect(43 + (PADDING+4)/2, 0, 60 - (PADDING + 4), 100, BLACK);
        showmsgXY(50, 18, 1, "STAT        INV        DATA        MAP        RADIO", GREEN, false);
      
        //Bottom bar
        int paddingforBottom = 10;
        int spacingBetweenboxes = 2;
        int h = 17;
        int wForSideBoxes = 480/4 - paddingforBottom * 2;
        tft.fillRect(paddingforBottom - spacingBetweenboxes, 300, wForSideBoxes, h, DARK_GREEN);
        tft.fillRect(480 - paddingforBottom - (wForSideBoxes) + spacingBetweenboxes, 300, wForSideBoxes, h, DARK_GREEN);
        tft.fillRect(paddingforBottom + wForSideBoxes, 300, 480 - 2 * wForSideBoxes - 2*paddingforBottom, h, DARK_GREEN);
        showmsgXY(paddingforBottom - spacingBetweenboxes + 2, 314, 1, "HP 100/100", GREEN, false);
        showmsgXY(480 - paddingforBottom - (wForSideBoxes) + spacingBetweenboxes + 20, 314, 1, "AP 70/70", GREEN, false);
        showmsgXY(paddingforBottom + wForSideBoxes + 85, 314, 1, "LEVEL 1", GREEN, false);
      }

      //SubText at the top
      showmsgXY(37, 46, 1, "STATUS", GREEN, false);
      showmsgXY(115, 46, 1, "SPECIAL", MEH_GREEN, false);
      showmsgXY(197, 46, 1, "PERKS", DARK_GREEN, false);

      //User name
      showmsgXY(480/2 - 40, 290, 1, "Nathan", GREEN, false);
    };
    void loadInv(void){
      uppertab = 2;

      tft.fillScreen(BLACK);
      tft.fillRect(0, 25, 480, 2, GREEN);
      tft.fillRect(130, 7, 43, 19, GREEN);
      tft.fillRect(130 + PADDING/2, 7 + PADDING - 2, 43 - PADDING, 19, BLACK);
      tft.fillRect(130 + (PADDING+4)/2, 0, 43 - (PADDING + 4), 100, BLACK);
      showmsgXY(50, 18, 1, "STAT        INV        DATA        MAP        RADIO", GREEN, false);
    };
    void loadData(void){
      uppertab = 3;

      tft.fillScreen(BLACK);
      tft.fillRect(0, 25, 480, 2, GREEN);
      tft.fillRect(200, 7, 60, 19, GREEN);
      tft.fillRect(200 + PADDING/2, 7 + PADDING - 2, 60 - PADDING, 19, BLACK);
      tft.fillRect(200 + (PADDING+4)/2, 0, 60 - (PADDING + 4), 100, BLACK);
      showmsgXY(50, 18, 1, "STAT        INV        DATA        MAP        RADIO", GREEN, false);
    };
    void loadMap(void){
      uppertab = 4;

      tft.fillScreen(BLACK);
      tft.fillRect(0, 25, 480, 2, GREEN);
      tft.fillRect(288, 7, 51, 19, GREEN);
      tft.fillRect(288 + PADDING/2, 7 + PADDING - 2, 51 - PADDING, 19, BLACK);
      tft.fillRect(288 + (PADDING+4)/2, 0, 51 - (PADDING + 4), 100, BLACK);
      showmsgXY(50, 18, 1, "STAT        INV        DATA        MAP        RADIO", GREEN, false);
    };
    void loadRadio(void){
      uppertab = 5;

      tft.fillScreen(BLACK);
      tft.fillRect(0, 25, 480, 2, GREEN);
      tft.fillRect(366, 7, 72, 19, GREEN);
      tft.fillRect(366 + PADDING/2, 7 + PADDING - 2, 72 - PADDING, 19, BLACK);
      tft.fillRect(366 + (PADDING+4)/2, 0, 72 - (PADDING + 4), 100, BLACK);
      showmsgXY(50, 18, 1, "STAT        INV        DATA        MAP        RADIO", GREEN, false);
    };
    void switchUpperTab(){
      if (uppertab == 1){
        loadStat();
      }else if (uppertab == 2){
        loadInv();
      }else if (uppertab == 3){
        loadData();
      }else if (uppertab == 4){
        loadMap();
      }else{
        loadRadio();
      } 
    }
    void switchLowerTab(void){
      if (uppertab == 1){
        tft.fillRect(0,27,sResX,sResY-50, BLACK);
        if (lowertab == 1){
          loadStat(false);
        }else if (lowertab == 2){
          int xOffset = 78;
          showmsgXY(37-xOffset, 46, 1, "STATUS", MEH_GREEN, false);
          showmsgXY(115-xOffset, 46, 1, "SPECIAL", GREEN, false);
          showmsgXY(197-xOffset, 46, 1, "PERKS", MEH_GREEN, false);
          // int prevX = 1;
          // for (int x = 1; x <= xOffset; x++){
          //   showmsgXY(37-x, 46, 1, "STATUS", MEH_GREEN, false);
          //   showmsgXY(115-x, 46, 1, "SPECIAL", GREEN, false);
          //   showmsgXY(197-x, 46, 1, "PERKS", MEH_GREEN, false);
          //   // delay(1); //time between each refresh to move over the text
          //   showmsgXY(37-prevX, 46, 1, "STATUS", BLACK, false);
          //   showmsgXY(115-prevX, 46, 1, "SPECIAL", BLACK, false);
          //   showmsgXY(197-prevX, 46, 1, "PERKS", BLACK, false);
            
          //   prevX = x;
          // }
          
        }else{
          int xOffset = 155;
          showmsgXY(37-xOffset, 46, 1, "STATUS", MEH_GREEN, false);
          showmsgXY(115-xOffset, 46, 1, "SPECIAL", MEH_GREEN, false);
          showmsgXY(197-xOffset, 46, 1, "PERKS", GREEN, false);
        }


      }
    }
};

Pipboy pipboy;

void setup(void){ 
  initialize_connections();
  // pipboy.startup();
  Serial.println("The Pipboy has finished starting up");
  // pipboy.switchUpperTab();
  // pipboy.loadRadio();
  pipboy.loadStat();
};

void loop(void){
  if (Serial.available() > 0){
    pipboy.lowertab = Serial.parseInt();
    pipboy.switchLowerTab();
  }
};
