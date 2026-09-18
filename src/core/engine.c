#include "engine.h"
#include <glad/gl.h>
#include "../platform/window.h"
#include "timer.h"
#include "logger.h"
#include "../platform/platform.h"
#include "../platform/input.h"
#include "frame_clock.h"

struct Engine{
  YVNEWindow *window;

  YVNE_Input *input;

  // Timer that starts at the beginning of the program;
  YVNE_Timer global_timer;

  // Timer that starts at the beginning of the game;
  YVNE_Timer game_timer;

  YVNE_FrameClock frame_clock;
  bool is_runnign;
};


Engine* YVNE_EngineInit(void){

  Engine *engine = calloc(1, sizeof *engine);
  if(engine == NULL){
	YVNE_LOG_ERROR("FAILED TO ALLOCATE MEMORY FOR ENGINE");
	return NULL;
  }

  if(!YVNE_SDLInit()){
	YVNE_LOG_ERROR("FAILED TO INITIALIZE SDL. SDL ERROR: %s", SDL_GetError());
	free(engine);
	return nullptr;
  }
  engine->is_runnign = true;

  engine->window = YVNE_WindowCreate(800, 600, "Kohaku Interpreter");
  if(!engine->window){
	YVNE_LOG_ERROR("FAILED TO ATTACH WINDOW TO ENGINE.");
	YVNE_SDLQuit();
	free(engine);
	return nullptr;
  }

  engine->input = YVNE_InputInit();
  if(!engine->input){
	YVNE_LOG_ERROR("FAILED TO ATTACH INPUT SYSTEM TO ENGINE!");
	YVNE_WindowDestroy(engine->window);
	YVNE_SDLQuit();
	free(engine);
	return nullptr;
  }

  const u64 global_time = SDL_GetTicksNS();
  YVNE_TimerInit(&engine->global_timer, global_time);
  //YVNE_TimerInit(&engine->game_timer, global_time);
  
  YVNE_FrameClockInit(&engine->frame_clock, global_time);

  YVNE_LOG_INFO("ENGINE INITIALIZATED SUCCESSFULLY");
  return engine;
}

bool YVNE_EngineRun(Engine *engine){
  YVNE_LOG_INFO("STARTING MAIN LOOP");
  while(engine->is_runnign == true){
	engine->global_timer.current_ticks = SDL_GetTicksNS();
	YVNE_InputOnFrame(engine->input);
	//u64 global_time = SDL_GetTicksNS();
	//u64 frame_delta = 
	SDL_Event event;
	while(SDL_PollEvent(&event)){
	  if(event.type == SDL_EVENT_QUIT) engine->is_runnign = false;
	  YVNE_InputHandle(engine->input, &event);
	}

	const YVNE_InputState *state = YVNE_InputGetState(engine->input);

	const u64 now = engine->global_timer.current_ticks;
	const double delta = (double)YVNE_FrameClockTick(&engine->frame_clock, now) / 1000000000.0;

	


    glClearColor(0.2f, 0.5f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
        
    SDL_GL_SwapWindow(engine->window->window);


  }
  return true;
}

void YVNE_EngineDestroy(Engine **engine){
  
  if(!engine || !*engine){
	YVNE_LOG_WARN("TRYING TO DESTROY EMTPY ENGINE STRUCTURE");
	return;
  }
  Engine *exemplar = *engine;
  if(exemplar->window){
	YVNE_InputDestroy(exemplar->input);
	YVNE_WindowDestroy(exemplar->window);
  }
  YVNE_SDLQuit();
  free(exemplar);
  *engine = NULL;
  YVNE_LOG_INFO("ENGINE DESTROYED SUCCESSFULLY");
}

