#include "stream.h"

#include <SDL3/SDL.h>

struct YVNE_Stream {
    SDL_IOStream *io;
    int64_t size;
};

YVNE_Stream *YVNE_StreamOpenFile(const char *path, u64 maximum_size)
{
    if (path == NULL) {
        SDL_SetError("stream path is null");
        return NULL;
    }

    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (io == NULL) {
        return NULL;
    }

    const Sint64 size = SDL_GetIOSize(io);
    if (size < 0 || (u64)size > maximum_size) {
        if (size >= 0) {
            SDL_SetError(
                "stream size %lld exceeds limit %llu",
                (long long)size,
                (unsigned long long)maximum_size
            );
        }
        SDL_CloseIO(io);
        return NULL;
    }

    YVNE_Stream *stream = SDL_malloc(sizeof *stream);
    if (stream == NULL) {
        SDL_SetError("failed to allocate stream");
        SDL_CloseIO(io);
        return NULL;
    }

    stream->io = io;
    stream->size = size;
    return stream;
}

YVNE_StreamReadResult YVNE_StreamRead(
    YVNE_Stream *stream,
    void *buffer,
    size_t size
) {
    if (stream == NULL || stream->io == NULL ||
        (buffer == NULL && size != 0)) {
        SDL_SetError("invalid stream read");
        return (YVNE_StreamReadResult){
            .status = YVNE_STREAM_READ_ERROR,
        };
    }

    if (size == 0) {
        return (YVNE_StreamReadResult){
            .status = YVNE_STREAM_READ_OK,
        };
    }

    const size_t bytes_read = SDL_ReadIO(stream->io, buffer, size);
    const SDL_IOStatus io_status = SDL_GetIOStatus(stream->io);
    YVNE_StreamReadStatus status = YVNE_STREAM_READ_OK;

    if (io_status == SDL_IO_STATUS_EOF) {
        status = YVNE_STREAM_READ_EOF;
    } else if (io_status != SDL_IO_STATUS_READY) {
        status = YVNE_STREAM_READ_ERROR;
    }

    return (YVNE_StreamReadResult){
        .bytes_read = bytes_read,
        .status = status,
    };
}

bool YVNE_StreamSeek(
    YVNE_Stream *stream,
    int64_t offset,
    YVNE_StreamOrigin origin,
    int64_t *new_position
) {
    if (stream == NULL || stream->io == NULL) {
        SDL_SetError("invalid stream seek");
        return false;
    }

    int64_t base;
    switch (origin) {
    case YVNE_STREAM_SEEK_BEGIN:
        base = 0;
        break;
    case YVNE_STREAM_SEEK_CURRENT:
        base = SDL_TellIO(stream->io);
        break;
    case YVNE_STREAM_SEEK_END:
        base = stream->size;
        break;
    default:
        SDL_SetError("invalid stream seek origin");
        return false;
    }

    if (base < 0 || base > stream->size ||
        (offset < 0 && offset < -base) ||
        (offset >= 0 && offset > stream->size - base)) {
        SDL_SetError("stream seek is outside bounds");
        return false;
    }

    const int64_t target = base + offset;
    const Sint64 position = SDL_SeekIO(stream->io, target, SDL_IO_SEEK_SET);
    if (position != target) {
        return false;
    }

    if (new_position != NULL) {
        *new_position = position;
    }
    return true;
}

int64_t YVNE_StreamGetSize(const YVNE_Stream *stream)
{
    return stream != NULL ? stream->size : -1;
}

SDL_IOStream *YVNE_StreamBorrowSDL(YVNE_Stream *stream)
{
    return stream != NULL ? stream->io : NULL;
}

void YVNE_StreamClose(YVNE_Stream **stream)
{
    if (stream == NULL || *stream == NULL) {
        return;
    }

    SDL_CloseIO((*stream)->io);
    SDL_free(*stream);
    *stream = NULL;
}
