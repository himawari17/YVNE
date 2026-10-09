#ifndef YVNE_STREAM_H
#define YVNE_STREAM_H

#include "../core/types.h"

#include <stdint.h>

typedef struct SDL_IOStream SDL_IOStream;
typedef struct YVNE_Stream YVNE_Stream;

typedef enum{
    YVNE_STREAM_SEEK_BEGIN,
    YVNE_STREAM_SEEK_CURRENT,
    YVNE_STREAM_SEEK_END,
} YVNE_StreamOrigin;

typedef enum{
    YVNE_STREAM_READ_OK,
    YVNE_STREAM_READ_EOF,
    YVNE_STREAM_READ_ERROR,
} YVNE_StreamReadStatus;

typedef struct{
    size_t bytes_read;
    YVNE_StreamReadStatus status;
} YVNE_StreamReadResult;

YVNE_Stream *YVNE_StreamOpenFile(const char *path, u64 maximum_size);
YVNE_StreamReadResult YVNE_StreamRead(
    YVNE_Stream *stream,
    void *buffer,
    size_t size
);
bool YVNE_StreamSeek(
    YVNE_Stream *stream,
    int64_t offset,
    YVNE_StreamOrigin origin,
    int64_t *new_position
);
int64_t YVNE_StreamGetSize(const YVNE_Stream *stream);
SDL_IOStream *YVNE_StreamBorrowSDL(YVNE_Stream *stream);
void YVNE_StreamClose(YVNE_Stream **stream);

#endif
