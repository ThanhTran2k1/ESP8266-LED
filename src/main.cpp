#include <Arduino.h>
#include <DMDESP.h>
// #include <fonts/ElektronMart6x8.h>
#include <fonts/DejaVuSansItalic9.h>
//----------------------------------------

//----------------------------------------DMD Configuration (P10 Panel)
#define DISPLAYS_WIDE 1 //--> Panel Columns
#define DISPLAYS_HIGH 1 //--> Panel Rows
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  //--> Number of Panels P10 used (Column, Row)
//----------------------------------------
static char *Text[] = {"123456789--"};

void Scrolling_Text(int y, uint8_t scrolling_speed) {
  static uint32_t pM;
  static uint32_t x;
  int width = Disp.width();
  Disp.setFont(DejaVuSansItalic9);
  int fullScroll = Disp.textWidth(Text[0]) + width;
  if((millis() - pM) > scrolling_speed) { 
    pM = millis();
    if (x < fullScroll) {
      ++x;
    } else {
      x = 0;
      return;
    }
    Disp.drawText(width - x, y, Text[0]);
  }  
}
//========================================================================VOID SETUP()
void setup() {
  //----------------------------------------DMDESP Setup
  Disp.start(); //--> Run the DMDESP library
  Disp.setBrightness(100); //--> Brightness level
  Disp.setFont(DejaVuSansItalic9); //--> Determine the font used
  //----------------------------------------
}
//========================================================================

//========================================================================VOID LOOP()
void loop() {
  Disp.loop(); //--> Run "Disp.loop" to refresh the LED
  Disp.drawText(0, -1, "1234567"); //--> Display text "Disp.drawText(x position, y position, text)"
  // Disp.drawText(0,9,"122334455");
 Scrolling_Text(8, 60); //--> Show running text "Scrolling_Text(y position, speed);"
}

