#ifndef BUFFER_BITS_H_INCLUDED
#define BUFFER_BITS_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct buffer
{
    uint32_t buffer;
    int pos;
    FILE *archivo;
} t_buffer_32_bits;

typedef struct lector_32_bits
{
    uint32_t buffer;
    int pos;
    int bits_restantes; // Cantidad de bits válidos en el buffer
    FILE *archivo;
} t_lector_32_bits;

t_buffer_32_bits *buffer__32_bits_new(FILE *f);

void buffer_32_escribir_bit(t_buffer_32_bits *buf, int bit);

void buffer_32_escribir_byte(t_buffer_32_bits *buf, unsigned char byte);

void vaciar_buffer(t_buffer_32_bits *buf);

t_lector_32_bits *lector_32_bits_new(FILE *f);

int lector_32_bits_leer_bit(t_lector_32_bits *lector);

#endif
