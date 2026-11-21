#include "utils.h"
#include "Arduino.h"

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