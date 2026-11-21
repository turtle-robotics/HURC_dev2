#ifndef TEAM_INFO_H
#define TEAM_INFO_H

#include <Arduino.h>
#include <Preferences.h>

const uint8_t broadcast_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


typedef struct TeamInfo {
    char team_name[32];
    uint8_t mac_addr[6];

    TeamInfo() : team_name{}, mac_addr {} {}

    TeamInfo(const char *new_team_name, const uint8_t *new_mac_addr) {
        strncpy(team_name, new_team_name, 31);
        team_name[sizeof(team_name) - 1] = '\0'; // ensure it is always truncated with null terminator

        for(int i = 0; i < 6; ++i) {
            mac_addr[i] = new_mac_addr[i];
        }

    }
} TeamInfo;


class TeamInfoStore {
private:
    //std::vector<TeamInfo> team_list;
    Preferences preferences;

public:
    TeamInfoStore();

    void load_defaults();

    size_t size();

    TeamInfo at(int idx);
    void push_back(TeamInfo new_team);

    void overwrite_list(TeamInfo *new_team_list, int count);

    
};




#endif