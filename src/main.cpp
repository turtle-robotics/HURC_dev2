#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <TurtleController.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

const int LOGO_START_X = 96;
const int LOGO_START_Y = 32;

// various constant data the controller uses
const uint8_t broadcast_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


const int address_count = 16; // total number of addresses in the list

const char* team_names[address_count] PROGMEM = {
  "debug test", // debug
  "Team 2", // 2
  "R.A.J.", // 4
  "Just Noticeable Diff.", // 6
  "Bestest Hatchling", // 7
  "Ball Robot", // 8
  "Awesome Team", // 12
  "RAPS", // 14
  "Faze Clan", // 18
  "what the sigma", // 19
  "Jailor-P.Diddy", // 20
  "crocs", // 21
  "Turtle", // 28
  "Team 29", // 29
  "A", // 31
  "SSM", // 33
};

const uint8_t address_list[address_count][6] PROGMEM = {
  {0x08, 0x00, 0x00, 0x00, 0x00, 0x00}, // debug
  {0xd4, 0xe9, 0xf4, 0xb5, 0x6d, 0xc4}, // 2
  {0xd4, 0xe9, 0xf4, 0xb3, 0xea, 0x88}, // 4
  {0xb0, 0xcb, 0xd8, 0xc0, 0x79, 0x3c}, // 6
  {0xd4, 0xe9, 0xf4, 0xb4, 0xa5, 0xc0}, // 7
  {0x80, 0xf3, 0xda, 0x42, 0xab, 0x8c}, // 8
  {0xd4, 0xe9, 0xf4, 0xb4, 0x11, 0x6c}, // 12
  {0xd4, 0xe9, 0xf4, 0xb3, 0x51, 0x94}, // 14
  {0xd4, 0xe9, 0xf4, 0xbc, 0xd3, 0xe0}, // 18
  {0xd4, 0xe9, 0xf4, 0xb3, 0x59, 0xf4}, // 19
  {0xec, 0xe3, 0x34, 0x22, 0xf7, 0x9c}, // 20
  {0xd4, 0xe9, 0xf4, 0xb3, 0xdf, 0x6c}, // 21
  {0x80, 0xf3, 0xda, 0x42, 0xa2, 0xa8}, // 28
  {0xd4, 0xe9, 0xf4, 0xb0, 0xe0, 0xec}, // 29
  {0x88, 0x57, 0x21, 0x94, 0x63, 0x00}, // 31
  {0x80, 0xf3, 0xda, 0x42, 0x95, 0xb8}, // 33
};

const int LOGO_WIDTH = 32;
const int LOGO_HEIGHT = 32;
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



// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float j1x; float j1y; bool j1z;
  float j2x; float j2y; bool j2z;

  bool butX; bool butY; bool butA; bool butB;
  bool butLb; bool butLt; bool butRb; bool butRt; bool butMenu; bool butDpad;
} struct_message;

// enum to track state declaration and increment operator
#pragma region state_enum_stuff
// Controller State enum
enum State{
  SEND_MODE,
  DEBUG_MODE
};
State& operator++(State& currentState){
  switch(currentState) {
    case SEND_MODE : return currentState = DEBUG_MODE;
    case DEBUG_MODE : return currentState = SEND_MODE;
    default : return currentState;
  }
  assert(false); // this should be unreachable, but if by some "miracle" the code makes it here it'll throw an error
  return currentState; // this is just to prevent the compiler for yelling at you

}

State operator++(State& currentState, int)
{
  State tmp(currentState);
  ++currentState;
  return tmp;
}
#pragma endregion state_enum_stuff

// ESP-now stuff
// Currently registered peer (minimizes 20 peer limit).
esp_now_peer_info_t activePeerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { // callback function when data sent
  /*Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  */ // uncomment this if want debug message spam
}

// Create a struct_message called controllerData
struct_message controllerData = {0, 0, 0, 0, 0, 0, 0, 0, 0};

// create an object to store last cycle's controllerData so we can see button pulses
struct_message lastControllerData = {0, 0, 0, 0, 0, 0, 0, 0, 0};;

// Create objects for controller and display
Controller controller;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_LENGTH, &Wire, OLED_RESET);

#pragma region function_declarations
// function declarations
void readMacAddress();
void updateData();

void drawDefaults(); // just has the following functions in 1
void drawTeamName();
void drawTurtleLogo();
void drawControllerBorders();

void drawControllerState(u_int startX = 30, u_int startY = 32);
void drawJoystickState(u_int startX, u_int startY);
void drawButtonState(u_int startX, u_int startY);

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
  drawDefaults();
  drawControllerState();
  display.display();

}



bool switchButtonPressed = false; // just so that we dont have to do controller.getS() a million times since that might reread the voltage every time, don't know honestly

void loop() {
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

  Serial.printf("joy1: %.2f %.2f %d  joy2: %.2f %.2f %d  buttons: %d %d %d %d %d  triggers: %d %d %d %d menu: %d\n", controllerData.j1x, controllerData.j1y,
    controllerData.j1z, controllerData.j2x, controllerData.j2y, controllerData.j2z, 
    controllerData.butX, controllerData.butY, controllerData.butA, controllerData.butB, 
    controllerData.butLt, controllerData.butLb, controllerData.butRt, controllerData.butRb, controllerData.butMenu, controllerData.butDpad); // uncomment if want debug messages

}

#pragma region drawing_functions

void drawDefaults(){
  drawTeamName();
  drawTurtleLogo();
  drawControllerBorders();
}

void drawTeamName(){
  display.setCursor(0,0);
  display.printf("#%d:%s", currentAddressIndex+1, team_names[currentAddressIndex]);
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
#pragma endregion drawing_functions

// Register the peer at address_list[index] with ESP-NOW
void activatePeer(int index){
  memcpy(activePeerInfo.peer_addr, address_list[index], 6);
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
  const uint8_t* mac = address_list[index];
  if(esp_now_is_peer_exist(mac)){
    if(esp_now_del_peer(mac) != ESP_OK){
      Serial.printf("deactivatePeer: failed to remove peer %d\n", index);
    }
  }
}

void sendingModeOperations(){

  if(getButtonRisingEdge(switchButtonPressed, lastSwitchButtonState)){ // if just swapped to sending mode
    activatePeer(currentAddressIndex); // register the selected peer
    display.clearDisplay();  // display new team info
    drawDefaults();
    drawControllerState();
    display.display();
  }
  
  drawControllerState();
  display.display();

  // fire in the hole
  esp_err_t result = esp_now_send(address_list[currentAddressIndex], (uint8_t *) &controllerData, sizeof(controllerData));


  /*if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }*/ // uncomment if want debug messages
}

void debugModeOperations(){
  if(getButtonRisingEdge(switchButtonPressed, lastSwitchButtonState)){
    deactivatePeer(currentAddressIndex); // drop the peer while in debug mode
    display.clearDisplay();
    drawDefaults();
    drawControllerState();
    //print debug indicator
    display.setCursor(0, 56);
    display.printf("DEBUG");
  }
  bool update = false; // track if update happened
  drawControllerState();
  

   if(getButtonRisingEdge(controllerData.butB, lastControllerData.butB)){ // Go forward a team when you press right bumper (butB for face plate)
    currentAddressIndex = (currentAddressIndex + 1) % address_count;
    update = true;
  }

  if(getButtonRisingEdge(controllerData.butA, lastControllerData.butA)){ // Go back a team when you press left bumper (butA for face plate)
    if(currentAddressIndex == 0){
      currentAddressIndex = address_count - 1;
    }
    else{
      currentAddressIndex = (currentAddressIndex - 1) % address_count;
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
    drawDefaults();
    drawControllerState();
    // print debug indicator
    display.setCursor(0, 56);
    display.printf("DEBUG");
  }

  display.display();
}

void drawControllerState(u_int startX, u_int startY){
  drawJoystick1State(startX - 31, startY);
  drawJoystick2State(startX, startY);
  drawButtonState(startX + 31, startY);
  drawControllerBorders();
}

bool getButtonRisingEdge(bool currentVal, bool oldVal){
  return currentVal && !oldVal; // return true only on rising edge of signal
}
