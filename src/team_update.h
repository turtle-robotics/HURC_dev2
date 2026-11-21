#ifndef TEAM_UPDATE_H
#define TEAM_UPDATE_H
#include <Arduino.h>
#include <team_info.h>


const uint8_t header[2] = {0xDE, 0xAD};

#define determine_team_count(len) ((len - sizeof(header)) / sizeof(TeamInfo) ) 











#endif