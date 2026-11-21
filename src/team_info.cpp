#include <Arduino.h>
#include "team_info.h"
#include <Preferences.h>


// various constant data the controller uses
const uint8_t broadcast_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const char team_namespace[] = "teams";

const int address_count = 11; // total number of addresses in the list

const char* team_names[address_count] PROGMEM = {
  "debug test", // debug
  "Matthew^2", // 5
  "IDC", // 7
  "Overclocked", // 13
  "spam musubi", // 15
  "Blind leading the blind", // 18
  "KZR", // 19
  "Forklift Certified", // 21
  "Placeholder", // 22
  "Among Us Imposters", // 23
  "Crazy Turtles", // 28
};

const uint8_t address_list[address_count][6] PROGMEM = {
  {0xec, 0xe3, 0x34, 0x21, 0x8c, 0xa0},
  {0xd4, 0xe9, 0xf4, 0xb3, 0xce, 0xc0}, // 5
  {0x58, 0x2a, 0xbd, 0x70, 0xf9, 0x54}, // 7
  {0x70, 0x4b, 0xca, 0x90, 0x43, 0xd0}, // 13
  {0x80, 0xf3, 0xda, 0x41, 0x45, 0xa4}, // 15
  {0x80, 0xf3, 0xda, 0x40, 0xdb, 0x88}, // 18
  {0x80, 0xf3, 0xda, 0x42, 0x7e, 0x30}, // 19
  {0x58, 0x2a, 0xbd, 0x70, 0xa0, 0xd8}, // 21
  {0x80, 0xf3, 0xda, 0x40, 0xe3, 0x14}, // 22
  {0x80, 0xf3, 0xda, 0x41, 0x68, 0x00}, // 23
  {0xd4, 0xe9, 0xf4, 0xb3, 0x29, 0x34}, // 28
};



TeamInfoStore::TeamInfoStore() : preferences(), team_count(address_count) {
    preferences.begin(team_namespace, false);
    
    for (int i = 0; i < address_count; ++i) {
        TeamInfo new_team_info(team_names[i], address_list[i]);
        
        std::string str_num = std::to_string(i);



        preferences.putBytes(str_num.c_str(), &new_team_info, sizeof(new_team_info));

    }
    preferences.end();

}


TeamInfo TeamInfoStore::at(int idx) {

    TeamInfo retrieved_team_info;

    preferences.begin(team_namespace, false);


    std::string str_num = std::to_string(idx);


    preferences.getBytes(str_num.c_str(), &retrieved_team_info, sizeof(retrieved_team_info));

    preferences.end();

    return retrieved_team_info;
}

size_t TeamInfoStore::size() {
    return team_count;
}

void TeamInfoStore::push_back(TeamInfo new_team) {
    preferences.begin(team_namespace, false);
    
    preferences.putBytes(std::to_string(team_count).c_str(), &new_team, sizeof(new_team));
    team_count++;
    preferences.end();
}

void TeamInfoStore::overwrite_list(TeamInfo *new_team_list, int count) {
    preferences.begin(team_namespace, false);
    preferences.clear();
    preferences.end();
    team_count = 0;


    for(int i = 0; i < count; ++i) {
        push_back(new_team_list[i]);
    }
    
}

