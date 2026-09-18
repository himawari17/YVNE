#include "platform.h"
#include <glad/gl.h>
#include "../core/logger.h"

void GetPlatformInfo(void);

bool YVNE_SDLInit(){
  if(!SDL_Init(SDL_INIT_VIDEO)){
	YVNE_LOG_ERROR("FAILED TO INIT SDL VIDEO %s", SDL_GetError());
	return false;
  }


  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#ifdef __APPLE__
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  YVNE_LOG_INFO("SDL AND OpenGL INITIALIZATED");
  return true;
}


void YVNE_SDLQuit(){
  SDL_Quit();
  YVNE_LOG_INFO("SDL SUBSYSTEMS DISABLED");
}

void YVNE_GetPlatformInfo(void){
  YVNE_LOG_INFO("Highest OpenGL Version: %s", glGetString(GL_VERSION));
  YVNE_LOG_INFO("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
  YVNE_LOG_INFO("Renderer Info: %s", glGetString(GL_RENDERER));
  YVNE_LOG_INFO("SDL Version: %d", SDL_GetVersion());
}

