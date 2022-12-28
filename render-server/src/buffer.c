#include "buffer.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

Buffer allocate_buffer(uint32_t num_bytes)
{
    Buffer buffer;
    memset(&buffer, 0, sizeof(buffer));

    buffer.data = (uint8_t*)(malloc(num_bytes));
    buffer.size = num_bytes;
    return buffer;
}

void deallocate_buffer(Buffer* buffer)
{
    if(buffer->data) {
        free(buffer->data);
        buffer->size = 0;
        buffer->data = NULL;
    }
}

void append_to_buffer(Buffer* buffer, void* source, int num_bytes)
{
    uint8_t* new_buffer = (uint8_t*) malloc(buffer->size + num_bytes);
    memcpy(new_buffer, buffer->data, buffer->size);
    memcpy(new_buffer + buffer->size, source, num_bytes);
    
    // Deallocate the previous buffer
    free(buffer->data);

    buffer->data  = new_buffer;
    buffer->size += num_bytes;
}
