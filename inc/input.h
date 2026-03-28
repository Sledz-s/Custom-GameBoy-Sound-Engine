/*  
      |               |
      |               |
      |   Input Lib   |
      |               |
      |               |
*/
#ifndef _INPUT_H
#define _INPUT_H
#include <gb/gb.h>

//-----------------------------------

extern uint8_t joy;
extern uint8_t prev_joy;

//-------------- JOYPAD -------------

#define JOY_LEFT    (joy & J_LEFT)
#define JOY_RIGHT   (joy & J_RIGHT)
#define JOY_UP      (joy & J_UP)
#define JOY_DOWN    (joy & J_DOWN)

#define JOY_A       (joy & J_A)
#define JOY_B       (joy & J_B)

#define JOY_START   (joy & J_START)

#define PRESSED_JOY_LEFT    (JOY_LEFT && !(prev_joy & J_LEFT))
#define PRESSED_JOY_RIGHT   (JOY_RIGHT && !(prev_joy & J_RIGHT))
#define PRESSED_JOY_UP      (JOY_UP && !(prev_joy & J_UP))
#define PRESSED_JOY_DOWN    (JOY_DOWN && !(prev_joy & J_DOWN))

#define PRESSED_JOY_A       (JOY_A && !(prev_joy & J_A))
#define PRESSED_JOY_B       (JOY_B && !(prev_joy & J_B))

//-----------------------------------

#endif