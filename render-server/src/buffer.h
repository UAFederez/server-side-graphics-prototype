#ifndef SSR_BUFFER_H
#define SSR_BUFFER_H

#include <stdint.h>

// Represents a simple implementation of a growable array
// TODO: In the future, add a capacity field to avoid excessive memory reallocation

typedef struct {
    uint8_t* data;
    uint32_t size;
} Buffer;

Buffer allocate_buffer(uint32_t num_bytes);
void append_to_buffer(Buffer* buffer, void* source, int num_bytes);
void deallocate_buffer(Buffer* buffer);

#endif // BUFFER_H

