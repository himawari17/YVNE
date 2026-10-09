#include "input.h"
#include "../core/types.h"
#include "../core/logger.h"

struct YVNE_Input{
  YVNE_InputState state;
  u32 buttons[YVNE_INPUT_COUNT];
};

static void YVNE_InputSetAction(YVNE_InputButton *button, bool down);

YVNE_Input* YVNE_InputInit(void){
  YVNE_Input *input = calloc(1, sizeof *input);
  if(!input){
	YVNE_LOG_ERROR("UNABLE TO ALLOCATE INPUT STRUCT!");
	return nullptr;
  }
  YVNE_LOG_INFO("INPUT SYSTEM READY");
  return input;
}
void YVNE_InputDestroy(YVNE_Input *input){
  if(!input){
	YVNE_LOG_WARN("TRYING TO DESTROY EMPTY INPUT STRUCT");
	return;
  }
  free(input);
  YVNE_LOG_INFO("INPUT SYSTEM DESTROYED");
}

void YVNE_InputOnFrame(YVNE_Input *input){
  if(!input){
	YVNE_LOG_WARN("TRYING TO USE UNINITIALIZED INPUT!");
	return;
  }
  for(u32 i = 0; i < YVNE_INPUT_COUNT; i++){
	input->state.events[i].pressed = false;
	input->state.events[i].released = false;
  }

  input->state.mouse.left.pressed = false;
  input->state.mouse.right.pressed = false;
  input->state.mouse.left.released = false;
  input->state.mouse.right.released = false;

  input->state.mouse.delta_x = 0.0f;
  input->state.mouse.delta_y = 0.0f;

}

void YVNE_InputHandle(YVNE_Input *input, const SDL_Event *event){
  if(!input || !event){
	YVNE_LOG_WARN("TRYING TO USE WHRONG INPUT SYSTEM OR EVENT!");
	return;
  }
  switch(event->type){
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:{
	  const bool down = event->type == SDL_EVENT_KEY_DOWN;
	  switch(event->key.scancode){
		case SDL_SCANCODE_SPACE:
		  YVNE_InputSetAction(&input->state.events[YVNE_INPUT_LEFT], down);
		  break;
		default: break;
	  }
	  break;
  }
	case SDL_EVENT_MOUSE_MOTION:
	  input->state.mouse.mouse_x = event->motion.x;
	  input->state.mouse.mouse_y = event->motion.y;
	  input->state.mouse.delta_x += event->motion.xrel;
	  input->state.mouse.delta_y += event->motion.yrel;
	  break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:{
	  input->state.mouse.mouse_x = event->button.x;
	  input->state.mouse.mouse_y = event->button.y;
	  const bool down = event->type == SDL_EVENT_MOUSE_BUTTON_DOWN;
	  if(event->button.button == SDL_BUTTON_LEFT){
		YVNE_InputSetAction(&input->state.mouse.left, down);
	  }
	  else if(event->button.button == SDL_BUTTON_RIGHT){
		YVNE_InputSetAction(&input->state.mouse.right, down);
	  }
	  break;
	}
	default: break;
  }
  
}

static void YVNE_InputSetAction(YVNE_InputButton *button, bool down){
  if(!button){
	YVNE_LOG_WARN("TRYING TO SET INPUT TO UNINITIALIZED BUTTON!");
	return;
  }
  if(button->down && !down){
	button->released = true;
  }
  if(!button->down && down){
	button->pressed = true;
  }

  button->down = down;
}

YVNE_InputState* YVNE_InputGetState(YVNE_Input *input){
  if(!input){
	YVNE_LOG_WARN("TRYING TO GET STATE OF UNINITIALIZED INPUT SYSTEM!");
	return nullptr;
  }
  return &input->state;

}


void YVNE_InputEndFrame(YVNE_Input *input);

void YVNE_InputKey(const YVNE_Input *input, SDL_Scancode key);
void YVNE_InputMouse(const YVNE_Input *input, Uint8 button);
