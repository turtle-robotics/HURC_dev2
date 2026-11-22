#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <TurtleController.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "utils.h"
#include "drawing_functions.h"
#include "team_info.h"
#include "team_update.h"
#include <vector>

// constants definitions
const int SCREEN_WIDTH = 128;
const int SCREEN_LENGTH = 64;
const int OLED_RESET = -1; // change this if the oled has a reset pin

const int joyL_x_pin = 34;
const int joyL_y_pin = 35;
const int joyL_z_pin = 25;

const int joyR_x_pin = 39;
const int joyR_y_pin = 36;
const int joyR_z_pin = 26;

const int but_a_pin = 17;
const int but_b_pin = 16;
const int but_x_pin = 19;
const int but_y_pin = 18;
const int but_Lt_pin = 27;
const int but_Lb_pin = 32;
const int but_Rt_pin = 13;
const int but_Rb_pin = 14;
const int but_menu_pin = 23;
const int but_dpad_pin = 12;

const int poll_rate = 64;
const float deadzone = 0.25f;




// ESP-now stuff
// Currently registered peer (minimizes 20 peer limit).
esp_now_peer_info_t activePeerInfo;

TeamInfoStore *team_infos;


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { // callback function when data sent
  /*Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  */ // uncomment this if want debug message spam
}

// ESP-NOW v1 supports at most 250 bytes of application payload.
constexpr size_t MAX_UPDATE_PACKET_SIZE = 250;
constexpr size_t TEAM_UPDATE_QUEUE_LENGTH = 12;

struct TeamUpdatePacket {
  uint16_t len;
  uint8_t data[MAX_UPDATE_PACKET_SIZE];
};

QueueHandle_t teamUpdateQueue = nullptr;

// Runs in the Wi-Fi task: copy only, never access Preferences here.
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len < HEADER_SIZE || len > static_cast<int>(MAX_UPDATE_PACKET_SIZE)) {
    return;
  }

  TeamUpdatePacket packet{};
  packet.len = static_cast<uint16_t>(len);
  memcpy(packet.data, incomingData, len);

  // Nonblocking: the Wi-Fi task must not wait for flash/NVS operations.
  if (teamUpdateQueue != nullptr) {
    xQueueSend(teamUpdateQueue, &packet, 0);
  }
}

// Runs in loop(), not the Wi-Fi callback.
void processTeamUpdates();

// Create a struct_message called controllerData
struct_message controllerData = {0, 0, 0, 0, 0, 0, 0, 0, 0};

// create an object to store last cycle's controllerData so we can see button pulses
struct_message lastControllerData = {0, 0, 0, 0, 0, 0, 0, 0, 0};

// Create objects for controller and display
Controller controller;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_LENGTH, &Wire, OLED_RESET);

#pragma region function_declarations
// function declarations
void readMacAddress();
void updateData();


void sendingModeOperations();
void debugModeOperations();
// function to only return true on rising edge
bool getButtonRisingEdge(bool currentVal, bool oldVal);
// peer management - add/remove the single active peer
void activatePeer(int index);
void deactivatePeer(int index);
#pragma endregion function_declarations

// runtime variables
int currentAddressIndex = 0;


bool lastSwitchButtonState = false;
enum State currentState = SEND_MODE;
bool flipX = true;
bool flipY = true;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin();

  // esp now setup
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while(true){
      Serial.println("im killing myself");
    }
  }
  teamUpdateQueue = xQueueCreate(TEAM_UPDATE_QUEUE_LENGTH, sizeof(TeamUpdatePacket));
  if (teamUpdateQueue == nullptr) {
    Serial.println("Failed to allocate team update queue!");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

 

  team_infos = new TeamInfoStore();
  if (team_infos->size() == 0) {
    Serial.println("using defaults!");
    team_infos->load_defaults();
  }
  else {
    Serial.println("loading stored team infos!");
  }

  // Register only the initially selected peer. Further add/remove is handled
  activatePeer(currentAddressIndex);
  
  // controller setup
  delay(1000);
  controller = Controller(joyL_x_pin, joyL_y_pin, joyL_z_pin, joyR_x_pin, joyR_y_pin, joyR_z_pin, but_x_pin, but_y_pin, but_a_pin, but_b_pin,
    but_Lt_pin, but_Lb_pin, but_Rt_pin, but_Rb_pin, but_menu_pin, but_dpad_pin);
 
  // OLED setup

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("oled don't work, killing self"); // inform that suicide is imminent
    while(true) {
      Serial.println("im killing myself");
    } // kill self
  }
  Serial.println("oled setup successful");

  // use oled time
  display.clearDisplay();
  // display configs
  display.setTextSize(1);      // normal font
  display.setTextColor(WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextWrap(true);
  
  // initial print to display
  drawDefaults(currentAddressIndex, team_infos->at(currentAddressIndex).team_name);
  drawControllerState();
  display.display();

}



bool switchButtonPressed = false; // just so that we dont have to do controller.getS() a million times since that might reread the voltage every time, don't know honestly

void loop() {
  processTeamUpdates();

  // loop code for stuff like button debouncing
  controller.controllerUpdate();
  // get all the updated controller data loaded
  updateData();
  switchButtonPressed = controller.getMenu();
  
  // switch device state if switch button gets pressed
  if(getButtonRisingEdge(switchButtonPressed, lastSwitchButtonState)){ // this is so we only switch once per button press
    ++currentState; // swap states
  }
  
  switch (currentState) // choose what to do based on current state
  {
  case SEND_MODE: sendingModeOperations(); break;
  case DEBUG_MODE: debugModeOperations(); break;
  default: break; // do nothing, this should be impossible to ever happen tho
  }
  
  // update last button states
  lastControllerData = controllerData;
  lastSwitchButtonState = switchButtonPressed;

  // delay according to polling rate
  delay((1.0f/poll_rate) * 1000);

}

void processTeamUpdates() {
  TeamUpdatePacket packet;
  while (xQueueReceive(teamUpdateQueue, &packet, 0) == pdTRUE) {
    const uint8_t *data = packet.data;
    const size_t len = packet.len;

    if (memcmp(data, const_header, sizeof(const_header)) != 0) {
      Serial.println("Invalid constant header!");
      continue;
    }

    const uint8_t type = data[sizeof(const_header)];
    if (type == type_clear) {
      if (len != HEADER_SIZE) {
        Serial.println("Invalid clear packet size!");
        continue;
      }
      // Remove the old peer before clearing the list it came from.
      if (team_infos->size() > 0 && currentState == SEND_MODE) {
        deactivatePeer(currentAddressIndex);
      }
      team_infos->clear();
      currentAddressIndex = 0;
      Serial.println("Cleared Teams!");
    } else if (type == type_new_team) {
      const size_t bytes = len - HEADER_SIZE;
      if (bytes == 0 || bytes % sizeof(TeamInfo) != 0) {
        Serial.println("Invalid team segment size!");
        continue;
      }
      const size_t count = bytes / sizeof(TeamInfo);
      for (size_t i = 0; i < count; ++i) {
        TeamInfo team;
        memcpy(&team, data + HEADER_SIZE + i * sizeof(TeamInfo), sizeof(team));
        team.team_name[sizeof(team.team_name) - 1] = '\0';
        team_infos->push_back(team);
      }
      Serial.printf("Updated teams: added %u!\n", static_cast<unsigned>(count));
      if (team_infos->size() > 0 && currentState == SEND_MODE) {
        activatePeer(currentAddressIndex);
      }
    } else {
      Serial.printf("Invalid variable header: 0x%02X\n", type);
      continue;
    }

    // Refresh the screen after a change, but don't index an empty list.
    display.clearDisplay();
    if (team_infos->size() > 0) {
      drawDefaults(currentAddressIndex, team_infos->at(currentAddressIndex).team_name);
    } else {
      display.setCursor(0, 0);
      display.print("Waiting for teams...");
    }
    if (team_infos->size() > 0) drawControllerState();
    if (currentState == DEBUG_MODE) {
      display.setCursor(0, 56);
      display.print("DEBUG");
    }
    display.display();
  }
}

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void updateData(){
  controllerData.j1x = controller.getJoy1X(deadzone) * ((flipX) ? -1 : 1); // flip x axis if flipX is true
  controllerData.j1y = controller.getJoy1Y(deadzone) * ((flipY) ? -1 : 1); // flip y axis if flipY is true
  controllerData.j1z = controller.getJoy1Z();

  controllerData.j2x = controller.getJoy2X(deadzone) * ((flipX) ? -1 : 1); // flip x axis if flipX is true
  controllerData.j2y = controller.getJoy2Y(deadzone) * ((flipY) ? -1 : 1); // flip y axis if flipY is true
  controllerData.j2z = controller.getJoy2Z();

  controllerData.butX = controller.getX();
  controllerData.butY = controller.getY();
  controllerData.butA = controller.getA();
  controllerData.butB = controller.getB();
  
  controllerData.butLt = controller.getLt();
  controllerData.butLb = controller.getLb();
  controllerData.butRt = controller.getRt();
  controllerData.butRb = controller.getRb();

  controllerData.butMenu = controller.getMenu();
  controllerData.butDpad = controller.getDpad();

  /*
  Serial.printf("joy1: %.2f %.2f %d  joy2: %.2f %.2f %d  buttons: %d %d %d %d %d  triggers: %d %d %d %d menu: %d\n", controllerData.j1x, controllerData.j1y,
    controllerData.j1z, controllerData.j2x, controllerData.j2y, controllerData.j2z, 
    controllerData.butX, controllerData.butY, controllerData.butA, controllerData.butB, 
    controllerData.butLt, controllerData.butLb, controllerData.butRt, controllerData.butRb, controllerData.butMenu, controllerData.butDpad); // uncomment if want debug messages
  */
}


// Register the peer at address_list[index] with ESP-NOW
void activatePeer(int index){
  memcpy(activePeerInfo.peer_addr, team_infos->at(index).mac_addr, 6);
  activePeerInfo.channel = 0;
  activePeerInfo.encrypt = false;
  if(!esp_now_is_peer_exist(activePeerInfo.peer_addr)){
    if(esp_now_add_peer(&activePeerInfo) != ESP_OK){
      Serial.printf("activatePeer: failed to add peer %d\n", index);
    }
  }
}

// Unregister the peer at address_list[index] from ESP-NOW
void deactivatePeer(int index){
  const uint8_t* mac = team_infos->at(index).mac_addr;
  if(esp_now_is_peer_exist(mac)){
    if(esp_now_del_peer(mac) != ESP_OK){
      Serial.printf("deactivatePeer: failed to remove peer %d\n", index);
    }
  }
}

void sendingModeOperations(){
  if (team_infos->size() == 0) return;

  if(getButtonRisingEdge(switchButtonPressed, lastSwitchButtonState)){ // if just swapped to sending mode
    activatePeer(currentAddressIndex); // register the selected peer
    display.clearDisplay();  // display new team info
    drawDefaults(currentAddressIndex, team_infos->at(currentAddressIndex).team_name);
    drawControllerState();
    display.display();
  }
  
  drawControllerState();
  display.display();

  // fire in the hole
  esp_err_t result = esp_now_send(team_infos->at(currentAddressIndex).mac_addr, (uint8_t *) &controllerData, sizeof(controllerData));


  /*if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }*/ // uncomment if want debug messages
}

void debugModeOperations(){
  if (team_infos->size() == 0) return;
  if(getButtonRisingEdge(switchButtonPressed, lastSwitchButtonState)){
    deactivatePeer(currentAddressIndex); // drop the peer while in debug mode
    display.clearDisplay();
    drawDefaults(currentAddressIndex, team_infos->at(currentAddressIndex).team_name);
    drawControllerState();
    //print debug indicator
    display.setCursor(0, 56);
    display.printf("DEBUG");
  }
  bool update = false; // track if update happened
  drawControllerState();
  

   if(getButtonRisingEdge(controllerData.butB, lastControllerData.butB)){ // Go forward a team when you press right bumper (butB for face plate)
    currentAddressIndex = (currentAddressIndex + 1) % team_infos->size();
    update = true;
  }

  if(getButtonRisingEdge(controllerData.butA, lastControllerData.butA)){ // Go back a team when you press left bumper (butA for face plate)
    if(currentAddressIndex == 0){
      currentAddressIndex = team_infos->size() - 1;
    }
    else{
      currentAddressIndex = (currentAddressIndex - 1) % team_infos->size();
    }
    update = true;
  }
  
  if(getButtonRisingEdge(controllerData.butX, lastControllerData.butX)){ // flip joystick x axis when press X
    flipX = !flipX;
  }

  if(getButtonRisingEdge(controllerData.butY, lastControllerData.butY)){ // flip joystick y axis when press Y
    flipY = !flipY;
  }

  if(update){
    display.clearDisplay();
    drawDefaults(currentAddressIndex, team_infos->at(currentAddressIndex).team_name);
    drawControllerState();
    // print debug indicator
    display.setCursor(0, 56);
    display.printf("DEBUG");
  }

  display.display();
}



bool getButtonRisingEdge(bool currentVal, bool oldVal){
  return currentVal && !oldVal; // return true only on rising edge of signal
}
