#include "rc4.h"
#include <stdlib.h>
#include <string.h>

typedef struct rc4_key_str
{
    unsigned char state[256];
    unsigned char x;
    unsigned char y;
} rc4_key;

static void swap_byte(unsigned char *a, unsigned char *b)
{
    unsigned char swapByte;

    swapByte = *a;
    *a = *b;
    *b = swapByte;
}

static void prepare_key(const unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
{
    unsigned char index1 = 0;
    unsigned char index2 = 0;
    unsigned char* state = NULL;
    short counter = 0;

    state = &key->state[0];
    for(counter = 0; counter < 256; counter++)
    {
        state[counter] = (unsigned char)counter;
    }

    key->x = 0;
    key->y = 0;
    for(counter = 0; counter < 256; counter++)
    {
        index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
        swap_byte(&state[counter], &state[index2]);

        index1 = (index1 + 1) % key_data_len;
    }
}

static void rc4(unsigned char *buffer_ptr, int *buffer_len,  rc4_key *key)
{
    unsigned char x;
    unsigned char y;
    unsigned char* state;
    unsigned char xorIndex;
    short counter;

    x = key->x;
    y = key->y;

    if(*buffer_len <= 0)
    {
        *buffer_len = strlen((char *)buffer_ptr);
    }

    state = &key->state[0];
    for(counter = 0; counter < *buffer_len; counter ++)
    {
        x = (x + 1) % 256;
        y = (state[x] + y) % 256;
        swap_byte(&state[x], &state[y]);

        xorIndex = (state[x] + state[y]) % 256;

        buffer_ptr[counter] ^= state[xorIndex];
    }

    key->x = x;
    key->y = y;
}

void rc4Decrypt(const char *key, char *data, int *data_len)
{
    rc4_key rc4key;
    prepare_key((const unsigned char*)key, strlen(key), &rc4key);
    rc4((unsigned char *)data, data_len, &rc4key);
}
