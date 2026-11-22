#ifndef DRAWING_FUNCTIONS_H
#define DRAWING_FUNCTIONS_H

#include "Arduino.h"

void drawDefaults(int team_index, const char* team_name); // just has the following functions in 1
void drawTeamName(int team_index, const char* team_name);

void drawTurtleLogo();
void drawControllerBorders();

void drawControllerState(u_int startX = 30, u_int startY = 32);
void drawJoystickState(u_int startX, u_int startY);
void drawButtonState(u_int startX, u_int startY);






#endif