#ifndef _FILE_STREAMING
#define _FILE_STREAMING
#include "../core/types.h"
#include <SDL3/SDL_iostream.h>
#include <stdlib.h>

typedef enum {
  YVNE_FILE_BEGIN,
  YVNE_FILE_CURRENT,
  YVNE_FILE_SEEK,
} YVNEFileState;

typedef struct {
  SDL_IOStream *io;
  int64_t size;
} YVNE_Stream;

typedef struct {
  size_t bytes_read;
  SDL_IOStatus status;
} YVNE_StreamReadResult;

size_t YVNE_StreamOpen(void *buffer, size_t size, YVNE_Stream *stream);

bool YVNE_StreamSeek(void *buffer, u64 offset, YVNE_Stream *stream);

size_t YVNE_StreamGetSize(YVNE_Stream *stream);

void YVNE_StreamClose(YVNE_Stream **stream);
#endif
