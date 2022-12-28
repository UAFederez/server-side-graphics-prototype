#ifndef SSR_SERVER_H
#define SSR_SERVER_H

#include <stdint.h>
#include "vector3f.h"
#include "buffer.h"

typedef struct {
    uint32_t image_width;
    uint32_t image_height;
    Vector3f light_pos;
    Vector3f sphere_pos;
} RenderRequest;

typedef struct {
    Buffer pixel_buffer;         // Just the flat array of each pixel color
    Buffer image_raw_bytes;      // Contains the image data in jpeg format
    Buffer image_base64_encoded; // Contains the string of the base64-encode image
} RenderResponse;

#endif // SERVER_H
