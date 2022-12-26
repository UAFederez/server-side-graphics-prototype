#ifndef BASIC_BASE64_FUNCTIONS_H
#define BASIC_BASE64_FUNCTIONS_H

#include <math.h>
#include <time.h>

static const char* BASE64_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Basic (and slow) implementation of base64 encoding:
char* base64_encode(const char* source, size_t len)
{
    size_t result_len  = ceil((float)len * 4.0f / 3.0f);
    size_t num_padding = (4 - (result_len % 4)) % 4;
    size_t string_len  = result_len + num_padding + 1;
    char*  result      = (char*) malloc(string_len);
    char*  current     = result;

    uint8_t bits_buffered     = 0;
    uint8_t num_bits_buffered = 0;

    memset(result, '=', string_len);
    result[string_len - 1] = '\0';
    for(size_t i = 0; i < len; i++)
    {
        const char* ch  = (source + i);
        uint8_t to_buff = ((i + 1)* 8) % 6;
        uint8_t mask    = 0xfc;    
        uint8_t shift   = 2;

        // If buffered 2 bits -> take next significant 4 (0xf0)
        // If buffered 4 bits -> take next significant 2 (0xc0)
        if(num_bits_buffered)
        {
            mask  = (num_bits_buffered / 2 - 1 ? 0xc0 : 0xf0);
            shift = (num_bits_buffered / 2 - 1 ? 6 : 4);
        }

        *(current++) = BASE64_TABLE[bits_buffered | ((mask & *ch) >> shift)];

        if(to_buff != 0)
        {
            bits_buffered     = ((~mask) & *ch) << (6 - to_buff);
            num_bits_buffered = to_buff;
        } else
        {
            // Means that there are 6 remaining least significant bits, 
            // no need to buffer, just add it to the string
            *(current++)      = BASE64_TABLE[ 0x3f & *ch ];
            bits_buffered     = 0;
            num_bits_buffered = 0;
        }
    }
    if(num_bits_buffered)
        *(current++) = BASE64_TABLE[bits_buffered | 0];
    return result;
}

char* base64_encode_fast(const char* source, size_t len)
{
    size_t result_len  = ceil((float)len * 4.0f / 3.0f);
    size_t num_padding = (4 - (result_len % 4)) % 4;
    size_t string_len  = result_len + num_padding + 1;
    char*  result      = (char*) malloc(string_len);
    char*  current     = result;

    if(!result)
    {
        printf("[Error] could not allocate enough memory!\n");
        return NULL;
    }

    memset(result, '=', string_len);
    result[string_len - 1] = '\0';

    size_t i = 0;
    for(; len - i >= 3; i += 3)
    {
        const char* ch  = (source + i);

        *(current++) = BASE64_TABLE[((0xfc & *ch) >> 2)];
        *(current++) = BASE64_TABLE[((0x03 & *ch++) << 4) | ((0xf0 & *ch) >> 4)];
        *(current++) = BASE64_TABLE[((0x0f & *ch++) << 2) | ((0xc0 & *ch) >> 6)];
        *(current++) = BASE64_TABLE[((0x3f & *ch))];
    }

    const char* ch = (source + i);
    int bytes_left = len - i;

    if(bytes_left)
    {
        *(current++) = BASE64_TABLE[((0xfc & *ch) >> 2)];

        if(bytes_left == 1)
            *(current++) = BASE64_TABLE[((0x03 & *ch++) << 4) | 0];
        else
        {
            *(current++) = BASE64_TABLE[((0x03 & *ch++) << 4) | ((0xf0 & *ch) >> 4)];
            *(current++) = BASE64_TABLE[((0x0f & *ch++) << 2) | 0];
        }
    }

    return result;
}

#endif
