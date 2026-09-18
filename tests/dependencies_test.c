#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glad/gl.h>

int main(void)
{
    return SDL_GetVersion() <= 0 || IMG_Version() <= 0 ||
           TTF_Version() <= 0 || MIX_Version() <= 0 ||
           GLAD_GL_VERSION_3_3 != 0;
}
