#ifndef UTILS_H
#define UTILS_H

typedef struct struct_message {
  float j1x; float j1y; bool j1z;
  float j2x; float j2y; bool j2z;

  bool butX; bool butY; bool butA; bool butB;
  bool butLb; bool butLt; bool butRb; bool butRt; bool butMenu; bool butDpad;
} struct_message;

// Controller State enum
enum State{
  SEND_MODE,
  DEBUG_MODE
};

State operator++(State& currentState, int);
State& operator++(State& currentState);


#endif