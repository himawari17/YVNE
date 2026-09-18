#ifndef _INPUT_H
#define _INPUT_H
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL.h>
#include <stdlib.h>

typedef struct{
  bool down;
  bool pressed;
  bool released;
} YVNE_InputButton;

typedef enum{
  YVNE_INPUT_LEFT,
  YVNE_INPUT_RIGHT,
  YVNE_INPUT_CANCEL,
  YVNE_INPUT_SKIP,
  YVNE_INPUT_COUNT,
} YVNEInputEvent;

typedef struct{
  float mouse_x;
  float mouse_y;
  float delta_x;
  float delta_y;

  YVNE_InputButton left;
  YVNE_InputButton right;

} YVNE_MouseState;


typedef struct{
  YVNE_InputButton events[YVNE_INPUT_COUNT];
  YVNE_MouseState mouse;
} YVNE_InputState;

typedef struct YVNE_Input YVNE_Input;

YVNE_Input* YVNE_InputInit(void);
void YVNE_InputDestroy(YVNE_Input *input);

// Function we call at the beginnnig of each frame
void YVNE_InputOnFrame(YVNE_Input *input);

void YVNE_InputHandle(YVNE_Input *input, const SDL_Event *event);

void YVNE_InputEndFrame(YVNE_Input *input);

YVNE_InputState* YVNE_InputGetState(YVNE_Input *input);

void YVNE_InputKey(const YVNE_Input *input, SDL_Scancode key);
void YVNE_InputMouse(const YVNE_Input *input, Uint8 button);


#endif
