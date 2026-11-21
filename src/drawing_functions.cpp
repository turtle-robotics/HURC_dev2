#include "drawing_functions.h"
#include "Arduino.h"
#include "Adafruit_SSD1306.h"
#include "utils.h"


extern Adafruit_SSD1306 display;
extern struct_message controllerData;
extern struct_message lastControllerData;


const int LOGO_WIDTH = 32;
const int LOGO_HEIGHT = 32;

const int LOGO_START_X = 96;
const int LOGO_START_Y = 32;
const unsigned char turtle_logo [] PROGMEM = { // sick af turtle logo
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xf0, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x38, 0xc1, 0xe0, 0x00, 0x09, 0x4b, 0xc0, 
	0x00, 0x01, 0x13, 0xc0, 0x00, 0x21, 0x97, 0x80, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x0f, 0xf9, 0x00, 
	0x00, 0x1f, 0xfe, 0x00, 0x00, 0x9f, 0xfc, 0xc0, 0x00, 0x7f, 0xfc, 0xc0, 0x00, 0x1f, 0xfc, 0x20, 
	0x07, 0x9f, 0xff, 0x80, 0x0f, 0x1f, 0xfc, 0x20, 0x0e, 0x7f, 0xf8, 0xe0, 0x06, 0x0f, 0xf8, 0x60, 
	0x00, 0x07, 0xe6, 0x60, 0x00, 0x04, 0x42, 0x60, 0x00, 0x09, 0x40, 0x20, 0x00, 0x03, 0x40, 0x20, 
	0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void drawDefaults(int team_index, const char* team_name){
  drawTeamName(team_index, team_name);
  drawTurtleLogo();
  drawControllerBorders();
}

void drawTeamName(int team_index, const char* team_name){
  display.setCursor(0,0);
  display.printf("#%d:%s", team_index + 1, team_name);
}

void drawTurtleLogo(){
  display.drawBitmap(LOGO_START_X, LOGO_START_Y, turtle_logo, LOGO_WIDTH, LOGO_HEIGHT, 1); // draw sick af turtle logo
}

void drawJoystick1State(u_int startX, u_int startY){
  // draw joystick state

  // some settings
  const u_int regionSize = 31;
  const u_int circleSize = 2;
  // used later
  const u_int maxDist = ((regionSize - 4)/2) - circleSize; // the minus 4 is for a nice 2 pixel border around the region just for spacing

  // calculate coordinates
  u_int CENTER_X = startX + regionSize/2;
  u_int CENTER_Y = startY + regionSize/2;

  u_int xCoord = CENTER_X + controllerData.j1x * maxDist;
  u_int yCoord = CENTER_Y + controllerData.j1y * maxDist;

  // draw
  display.fillCircle(CENTER_X + lastControllerData.j1x * maxDist, CENTER_Y + lastControllerData.j1y * maxDist, circleSize, BLACK); // wipe last
  if(controllerData.j1z)
    display.fillCircle(xCoord, yCoord, 2, WHITE);
  else
    display.drawCircle(xCoord, yCoord, 2, WHITE);

}

void drawJoystick2State(u_int startX, u_int startY){
  // draw joystick state

  // some settings
  const u_int regionSize = 31;
  const u_int circleSize = 2;
  // used later
  const u_int maxDist = ((regionSize - 4)/2) - circleSize; // the minus 4 is for a nice 2 pixel border around the region just for spacing

  // calculate coordinates
  u_int CENTER_X = startX + regionSize/2;
  u_int CENTER_Y = startY + regionSize/2;

  u_int xCoord = CENTER_X + controllerData.j2x * maxDist;
  u_int yCoord = CENTER_Y + controllerData.j2y * maxDist;

  // draw
  display.fillCircle(CENTER_X + lastControllerData.j2x * maxDist, CENTER_Y + lastControllerData.j2y * maxDist, circleSize, BLACK); // wipe last
  if(controllerData.j2z)
    display.fillCircle(xCoord, yCoord, 2, WHITE);
  else
    display.drawCircle(xCoord, yCoord, 2, WHITE);

}

void drawButtonState(u_int startX, u_int startY){
  // some settings
  const u_int regionXSize = 35;
  const u_int regionYSize = 32;
  
  const u_int buttonRadius = 2;

  // calculate coordinates
  u_int centerX = startX + regionXSize/2;
  u_int centerY = startY + regionYSize/2 - regionYSize/13;

    // Lb button
    u_int LbButtonStartX = centerX - regionXSize/5 - 3;
    u_int LbButtonStartY = centerY - regionYSize/5 - 2;

    // Rb button
    u_int RbButtonStartX = centerX + regionXSize/5 - 3;
    u_int RbButtonStartY = centerY - regionYSize/5 - 2;

    // Lt button
    u_int LtButtonStartX = centerX - regionXSize/5 - 2;
    u_int LtButtonStartY = centerY - regionYSize/5 - 7;

    // Rt button
    u_int RtButtonStartX = centerX + regionXSize/5 - 2;
    u_int RtButtonStartY = centerY - regionYSize/5 - 7;

    // A button
    u_int AButtonStartX = centerX;
    u_int AButtonStartY = centerY + regionXSize/17 + regionYSize/3;

    // B button
    u_int BButtonStartX = centerX + regionXSize/4;
    u_int BButtonStartY = centerY + (regionXSize/17 + regionYSize/3)/2 + 1;

    // X button
    u_int XButtonStartX = centerX - regionXSize/4;
    u_int XButtonStartY = centerY + (regionXSize/17 + regionYSize/3)/2 + 1;

    // Y button
    u_int YButtonStartX = centerX;
    u_int YButtonStartY = centerY + regionXSize/17;

    // Dpad Button
    u_int DpadButtonStartX = centerX - regionXSize - 16; // idk if theres a better way to do this but whatever
    u_int DpadButtonStartY = centerY - regionYSize/3 - 10;

  // draw Lb button
  if(controllerData.butLb){
    display.fillRoundRect(LbButtonStartX, LbButtonStartY, 8, 4, 2, WHITE);
  }
  else{
    display.fillRoundRect(LbButtonStartX, LbButtonStartY, 8, 4, 2, BLACK);
    display.drawRoundRect(LbButtonStartX, LbButtonStartY, 8, 4, 2, WHITE);
  }

  // draw Rb button
  if(controllerData.butRb){
    display.fillRoundRect(RbButtonStartX, RbButtonStartY, 8, 4, 2, WHITE);
  }
  else{
    display.fillRoundRect(RbButtonStartX, RbButtonStartY, 8, 4, 2, BLACK);
    display.drawRoundRect(RbButtonStartX, RbButtonStartY, 8, 4, 2, WHITE);
  }

  // draw Lt button
  if(controllerData.butLt){
    display.fillRoundRect(LtButtonStartX, LtButtonStartY, 6, 6, 2, WHITE);
  }
  else{
    display.fillRoundRect(LtButtonStartX, LtButtonStartY, 6, 6, 2, BLACK);
    display.drawRoundRect(LtButtonStartX, LtButtonStartY, 6, 6, 2, WHITE);
  }
  
  // draw Rt button
  if(controllerData.butRt){
    display.fillRoundRect(RtButtonStartX, RtButtonStartY, 6, 6, 2, WHITE);
  }
  else{
    display.fillRoundRect(RtButtonStartX, RtButtonStartY, 6, 6, 2, BLACK);
    display.drawRoundRect(RtButtonStartX, RtButtonStartY, 6, 6, 2, WHITE);
  }

  // draw A button
  if(controllerData.butA){
    display.fillCircle(AButtonStartX, AButtonStartY, buttonRadius, WHITE);
  }
  else{
    display.fillCircle(AButtonStartX, AButtonStartY, buttonRadius, BLACK);
    display.drawCircle(AButtonStartX, AButtonStartY, buttonRadius, WHITE);
  }

  // draw B button
  if(controllerData.butB){
    display.fillCircle(BButtonStartX, BButtonStartY, buttonRadius, WHITE);
  }
  else{
    display.fillCircle(BButtonStartX, BButtonStartY, buttonRadius, BLACK);
    display.drawCircle(BButtonStartX, BButtonStartY, buttonRadius, WHITE);
  }

  // draw X button
  if(controllerData.butX){
    display.fillCircle(XButtonStartX, XButtonStartY, buttonRadius, WHITE);
  }
  else{
    display.fillCircle(XButtonStartX, XButtonStartY, buttonRadius, BLACK);
    display.drawCircle(XButtonStartX, XButtonStartY, buttonRadius, WHITE);
  }

  // draw Y button
  if(controllerData.butY){
    display.fillCircle(YButtonStartX, YButtonStartY, buttonRadius, WHITE);
  }
  else{
    display.fillCircle(YButtonStartX, YButtonStartY, buttonRadius, BLACK);
    display.drawCircle(YButtonStartX, YButtonStartY, buttonRadius, WHITE);
  }

  // draw Dpad button
  if(controllerData.butDpad){
    display.fillRoundRect(DpadButtonStartX, DpadButtonStartY, 6, 4, 2, WHITE);
  }
  else{
    display.fillRoundRect(DpadButtonStartX, DpadButtonStartY, 6, 4, 2, BLACK);
    display.drawRoundRect(DpadButtonStartX, DpadButtonStartY, 6, 4, 2, WHITE);
  }
}

void drawControllerBorders(){
  display.drawRoundRect(1, 34, 27, 27, 2, WHITE); // draw border around joystick 1
  display.drawRoundRect(32, 34, 27, 27, 2, WHITE); // draw border around joystick 2
}

void drawControllerState(u_int startX, u_int startY){
  drawJoystick1State(startX - 31, startY);
  drawJoystick2State(startX, startY);
  drawButtonState(startX + 31, startY);
  drawControllerBorders();
}