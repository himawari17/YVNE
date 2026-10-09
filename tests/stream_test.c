#include "../src/resource/stream.h"

#include <SDL3/SDL.h>
#include <assert.h>
#include <string.h>

int main(void)
{
    static const char contents[] = "YVNE test asset";
    const int64_t size = sizeof contents - 1;
    const char *path = YVNE_TEST_PROJECT_DIR "/assets/sample.txt";
    assert(YVNE_StreamOpenFile(NULL, UINT64_MAX) == NULL);
    assert(YVNE_StreamOpenFile(YVNE_TEST_PROJECT_DIR "/missing.txt", UINT64_MAX) == NULL);
    assert(YVNE_StreamOpenFile(path, size - 1) == NULL);
    assert(YVNE_StreamOpenFile(path, 0) == NULL);
    YVNE_Stream *stream = YVNE_StreamOpenFile(path, size);
    assert(stream != NULL);
    assert(YVNE_StreamGetSize(stream) == size);
    assert(SDL_GetIOSize(YVNE_StreamBorrowSDL(stream)) == size);

    YVNE_StreamReadResult read = YVNE_StreamRead(stream, NULL, 0);
    assert(read.bytes_read == 0 && read.status == YVNE_STREAM_READ_OK);
    read = YVNE_StreamRead(stream, NULL, 1);
    assert(read.bytes_read == 0 && read.status == YVNE_STREAM_READ_ERROR);
    char buffer[32] = {0};
    read = YVNE_StreamRead(stream, buffer, 4);
    assert(read.bytes_read == 4 && read.status == YVNE_STREAM_READ_OK);
    assert(memcmp(buffer, contents, 4) == 0);
    int64_t position = -1;
    assert(YVNE_StreamSeek(stream, -2, YVNE_STREAM_SEEK_CURRENT, &position));
    assert(position == 2);
    assert(YVNE_StreamSeek(stream, -1, YVNE_STREAM_SEEK_END, &position));
    assert(position == size - 1);
    read = YVNE_StreamRead(stream, buffer, sizeof buffer);
    assert(read.bytes_read == 1 && read.status == YVNE_STREAM_READ_EOF);
    assert(buffer[0] == 't');

    assert(YVNE_StreamSeek(stream, 0, YVNE_STREAM_SEEK_BEGIN, &position));
    assert(position == 0);
    assert(!YVNE_StreamSeek(stream, INT64_MIN, YVNE_STREAM_SEEK_BEGIN, &position));
    assert(!YVNE_StreamSeek(stream, INT64_MAX, YVNE_STREAM_SEEK_CURRENT, &position));
    assert(!YVNE_StreamSeek(stream, 1, YVNE_STREAM_SEEK_END, &position));
    assert(!YVNE_StreamSeek(stream, 0, (YVNE_StreamOrigin)99, &position));
    assert(position == 0);
    read = YVNE_StreamRead(stream, buffer, sizeof buffer);
    assert(read.bytes_read == (size_t)size && read.status == YVNE_STREAM_READ_EOF);
    assert(memcmp(buffer, contents, size) == 0);
    YVNE_StreamClose(&stream);
    assert(stream == NULL);
    YVNE_StreamClose(&stream);
    YVNE_StreamClose(NULL);
    assert(YVNE_StreamGetSize(NULL) == -1);
    assert(YVNE_StreamBorrowSDL(NULL) == NULL);
    assert(!YVNE_StreamSeek(NULL, 0, YVNE_STREAM_SEEK_BEGIN, NULL));
    read = YVNE_StreamRead(NULL, buffer, 1);
    assert(read.bytes_read == 0 && read.status == YVNE_STREAM_READ_ERROR);

    stream = YVNE_StreamOpenFile(YVNE_TEST_PROJECT_DIR "/assets/empty.txt", 0);
    assert(stream != NULL && YVNE_StreamGetSize(stream) == 0);
    assert(YVNE_StreamSeek(stream, 0, YVNE_STREAM_SEEK_END, &position));
    assert(position == 0);
    read = YVNE_StreamRead(stream, buffer, 1);
    assert(read.bytes_read == 0 && read.status == YVNE_STREAM_READ_EOF);
    YVNE_StreamClose(&stream);
    return 0;
}
