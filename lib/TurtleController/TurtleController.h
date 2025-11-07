#ifndef TURTLE_CONTROLLER
#define TURTLE_CONTROLLER

#include <Arduino.h>
#include <ezButton.h>

class Joystick{
    private:
        int xPin, yPin;
        ezButton zButton;

    public:
        Joystick(); // default constructor
        Joystick(int xPin, int yPin, int zPin); // constructor
        ~Joystick(); // destructor
        //Joystick(const Joystick& other); // copy constructor
        //Joystick& operator=(const Joystick& other); // copy assignment operator

        float getX(float deadzone);
        float getY(float deadzone);
        bool getZ();
        void joystickUpdate();
    private:
        void setup();

};


class Controller{
    private:
        Joystick joy1, joy2; // left and right joysticks respectively
        ezButton butA, butB, butX, butY, butLt, butLb, butRt, butRb, butMenu, butDpad;
    
    public:
        Controller(); // default constructor
        Controller(int j1x, int j1y, int j1z, int j2x, int j2y, int j2z, int butXPin, int butYPin, int butAPin, int butBPin, int butLtPin, int butLbPin, int butRtPin, int butRbPin, int butMenuPin, int butDpadPin); // two-joystick constructor
        ~Controller();

        // Left / Joy1
        float getJoy1X(float deadzone = 0);
        float getJoy1Y(float deadzone = 0);
        bool getJoy1Z();

        // Right / Joy2
        float getJoy2X(float deadzone = 0);
        float getJoy2Y(float deadzone = 0);
        bool getJoy2Z();

        bool getA();
        bool getB();
        bool getX();
        bool getY();
        bool getLt();
        bool getLb();
        bool getRt();
        bool getRb();
        bool getMenu();
        bool getDpad();

        void controllerUpdate();

    private:
        void setup();
};

#endif