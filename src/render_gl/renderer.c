#include "renderer.h"
#include "../core/logger.h"

#include <math.h>
#include <stddef.h>

static const char *vertex_source =
    "#version 330 core\n"
    "layout(location = 0) in vec2 a_position;\n"
    "layout(location = 1) in vec2 a_uv;\n"
    "uniform vec2 u_canvas_size;\n"
    "out vec2 v_uv;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec2 position = vec2(\n"
    "        2.0 * a_position.x / u_canvas_size.x - 1.0,\n"
    "        1.0 - 2.0 * a_position.y / u_canvas_size.y\n"
    "    );\n"
    "    gl_Position = vec4(position, 0.0, 1.0);\n"
    "    v_uv = a_uv;\n"
    "}\n";
static const char *fragment_source =
    "#version 330 core\n"
    "in vec2 v_uv;\n"
    "uniform sampler2D u_texture;\n"
    "out vec4 fragment_color;\n"
    "void main()\n"
    "{\n"
    "   fragment_color = texture(u_texture, v_uv);\n"
    "}\n";

struct YVNE_Renderer {
  GLuint program;
  GLuint vao;
  GLuint vbo;

  int canvas_width;
  int canvas_height;
  GLint max_viewport[2];
  YVNE_Viewport viewport;
#ifdef YVNE_DEBUG
  int logged_framebuffer_width, logged_framebuffer_height;
  Uint64 viewport_log_ticks;
#endif
};

typedef struct {
  float x, y;
  float u, v;
} YVNE_SpriteVertex;

static GLuint YVNE_CompileShader(GLenum type, const char *source) {
  const GLuint shader = glCreateShader(type);
  if (!shader) {
    YVNE_LOG_ERROR("UNABLE TO CREATE SHADER. GL ERROR: 0x%x", glGetError());
    return 0;
  }
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    char log[1024] = {0};
    glGetShaderInfoLog(shader, sizeof log, NULL, log);
    YVNE_LOG_ERROR("SHADER COMPILATION FAILED: %s", log);
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

YVNE_Renderer *YVNE_GLRendererCreate(int canvas_width, int canvas_height) {
  if (canvas_width <= 0 || canvas_height <= 0 || !SDL_GL_GetCurrentContext() ||
      !GLAD_GL_VERSION_3_3) {
    YVNE_LOG_ERROR("INVALID CANVAS SIZE OR NO OPENGL 3.3 CONTEXT");
    return NULL;
  }
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    YVNE_LOG_ERROR("GL ERROR BEFORE RENDERER CREATION: 0x%x", error);
    return NULL;
  }
  YVNE_Renderer *renderer = SDL_calloc(1, sizeof(YVNE_Renderer));
  if (!renderer) {
    YVNE_LOG_ERROR("UNABLE TO ALLOCATE GL RENDERER!");
    return NULL;
  }
  renderer->canvas_width = canvas_width;
  renderer->canvas_height = canvas_height;

  const GLuint vertex_shader =
      YVNE_CompileShader(GL_VERTEX_SHADER, vertex_source);
  if (!vertex_shader) {
    goto fail;
  }
  const GLuint fragment_shader =
      YVNE_CompileShader(GL_FRAGMENT_SHADER, fragment_source);
  if (!fragment_shader) {
    glDeleteShader(vertex_shader);
    goto fail;
  }
  renderer->program = glCreateProgram();
  if (!renderer->program) {
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    YVNE_LOG_ERROR("UNABLE TO CREATE SHADER PROGRAM. GL ERROR: 0x%x",
                   glGetError());
    goto fail;
  }
  glAttachShader(renderer->program, vertex_shader);
  glAttachShader(renderer->program, fragment_shader);
  glLinkProgram(renderer->program);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  GLint linked;
  glGetProgramiv(renderer->program, GL_LINK_STATUS, &linked);
  if (!linked) {
    char log[1024] = {0};
    glGetProgramInfoLog(renderer->program, sizeof log, NULL, log);
    YVNE_LOG_ERROR("SHADER PROGRAM LINK FAILED: %s", log);
    goto fail;
  }

  const GLint texture_location =
      glGetUniformLocation(renderer->program, "u_texture");
  const GLint canvas_location =
      glGetUniformLocation(renderer->program, "u_canvas_size");
  if (texture_location < 0 || canvas_location < 0) {
    YVNE_LOG_ERROR("REQUIRED SHADER UNIFORM IS MISSING");
    goto fail;
  }
  glUseProgram(renderer->program);
  glUniform1i(texture_location, 0);
  glUniform2f(canvas_location, (float)canvas_width, (float)canvas_height);

  glGenVertexArrays(1, &renderer->vao);
  glGenBuffers(1, &renderer->vbo);

  glBindVertexArray(renderer->vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(YVNE_SpriteVertex), NULL,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(YVNE_SpriteVertex),
                        (const void *)offsetof(YVNE_SpriteVertex, x));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(YVNE_SpriteVertex),
                        (const void *)offsetof(YVNE_SpriteVertex, u));

  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, renderer->max_viewport);
  error = glGetError();
  if (!renderer->vao || !renderer->vbo || error != GL_NO_ERROR) {
    YVNE_LOG_ERROR("RENDERER INITIALIZATION FAILED. GL ERROR: 0x%x", error);
    goto fail;
  }

  YVNE_LOG_INFO("GL RENDERER READY: CANVAS %dx%d", canvas_width, canvas_height);
  return renderer;

fail:
  YVNE_GLRendererDestroy(&renderer);
  return NULL;
}

void YVNE_GLRendererDraw(YVNE_Renderer *renderer, const YVNE_GLTexture *texture,
                         float x, float y, float width, float height) {
  if (!renderer || !texture || !texture->handle ||
      renderer->viewport.width <= 0 || renderer->viewport.height <= 0 ||
      !isfinite(x) || !isfinite(y) || !isfinite(width) || !isfinite(height) ||
      !isfinite(x + width) || !isfinite(y + height) || width <= 0.0f ||
      height <= 0.0f) {
    YVNE_LOG_ERROR("INVALID SPRITE OR NO DRAWABLE FRAME");
    return;
  }
  const YVNE_SpriteVertex sprite_vertices[6] = {
      {x, y, 0.0f, 0.0f},
      {x + width, y, 1.0f, 0.0f},
      {x, y + height, 0.0f, 1.0f},
      {x + width, y, 1.0f, 0.0f},
      {x + width, y + height, 1.0f, 1.0f},
      {x, y + height, 0.0f, 1.0f},
  };
  glUseProgram(renderer->program);
  glBindVertexArray(renderer->vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sprite_vertices), sprite_vertices);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture->handle);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool YVNE_GLRendererOnFrame(YVNE_Renderer *renderer, int framebuffer_width,
                            int framebuffer_height) {
  if (!renderer ||
      !YVNE_ViewportCalculate(&renderer->viewport, renderer->canvas_width,
                              renderer->canvas_height, framebuffer_width,
                              framebuffer_height)) {
    return false;
  }

  const YVNE_Viewport *viewport = &renderer->viewport;
  if (viewport->width > renderer->max_viewport[0] ||
      viewport->height > renderer->max_viewport[1]) {
    YVNE_LOG_ERROR("VIEWPORT EXCEEDS OPENGL LIMITS");
    renderer->viewport = (YVNE_Viewport){0};
    return false;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(viewport->x, framebuffer_height - viewport->y - viewport->height,
             viewport->width, viewport->height);

  const GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    YVNE_LOG_ERROR("UNABLE TO BEGIN FRAME. GL ERROR: 0x%x", error);
    renderer->viewport = (YVNE_Viewport){0};
    return false;
  }

#ifdef YVNE_DEBUG
  if (framebuffer_width != renderer->logged_framebuffer_width ||
      framebuffer_height != renderer->logged_framebuffer_height) {
    const Uint64 now = SDL_GetTicks();
    // Coalesce resize events so dragging the window does not flood the log.
    if (renderer->logged_framebuffer_width == 0 ||
        now - renderer->viewport_log_ticks >= 1000) {
      YVNE_LOG_INFO("VIEWPORT UPDATED: FRAMEBUFFER %dx%d, VIEWPORT %dx%d AT (%d, %d)",
                     framebuffer_width, framebuffer_height,
                     viewport->width, viewport->height, viewport->x, viewport->y);
      renderer->logged_framebuffer_width = framebuffer_width;
      renderer->logged_framebuffer_height = framebuffer_height;
      renderer->viewport_log_ticks = now;
    }
  }
#endif

  return true;
}

const YVNE_Viewport *YVNE_GLRendererGetViewport(const YVNE_Renderer *renderer) {
  return renderer ? &renderer->viewport : NULL;
}

void YVNE_GLRendererDestroy(YVNE_Renderer **renderer) {
  if (!renderer || !*renderer) {
    return;
  }
  if ((*renderer)->program) {
    GLint current_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
    if ((GLuint)current_program == (*renderer)->program) {
      glUseProgram(0);
    }
    glDeleteProgram((*renderer)->program);
  }
  if ((*renderer)->vao) {
    glDeleteVertexArrays(1, &(*renderer)->vao);
  }
  if ((*renderer)->vbo) {
    glDeleteBuffers(1, &(*renderer)->vbo);
  }
  SDL_free(*renderer);
  *renderer = NULL;
  YVNE_LOG_INFO("GL RENDERER RESOURCES RELEASED");
}
