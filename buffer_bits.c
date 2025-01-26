#include "buffer_bits.h"

static void buffer_reset(t_buffer_32_bits *buf);
static int buffer_esta_completo(t_buffer_32_bits *buf);

t_buffer_32_bits *buffer__32_bits_new(FILE *f)
{
    t_buffer_32_bits *buf = malloc(sizeof(t_buffer_32_bits));
    if (!buf || !f)
        return NULL;
    buf->buffer = 0x00;
    buf->pos = 0;
    buf->archivo = f;
    return buf;
}

static void escribir_buffer_en_archivo(t_buffer_32_bits *buf)
{
    fwrite(&buf->buffer, sizeof(uint32_t), 1, buf->archivo);
}

void buffer_32_escribir_bit(t_buffer_32_bits *buf, int bit)
{
    buf->buffer |= (bit << (31 - buf->pos));
    buf->pos++;
    if (buf->pos == 32)
    {
        escribir_buffer_en_archivo(buf);
        buffer_reset(buf);
    }
}

void buffer_32_escribir_byte(t_buffer_32_bits *buf, unsigned char byte)
{
    for (int i = 7; i >= 0; i--)
    {
        buffer_32_escribir_bit(buf, (byte >> i) & 1);
    }
}

void vaciar_buffer(t_buffer_32_bits *buf)
{
    if (!buffer_esta_completo(buf))
    {
        escribir_buffer_en_archivo(buf);
    }
}

static int buffer_esta_completo(t_buffer_32_bits *buf)
{
    return buf->pos == 0;
}

static void buffer_reset(t_buffer_32_bits *buf)
{
    buf->buffer = 0x00;
    buf->pos = 0;
}

t_lector_32_bits *lector_32_bits_new(FILE *f)
{
    t_lector_32_bits *lector = malloc(sizeof(t_lector_32_bits));
    if (!lector || !f)
        return NULL;

    lector->buffer = 0x00;
    lector->pos = 0;
    lector->bits_restantes = 0;
    lector->archivo = f;
    return lector;
}

static int lector_32_bits_cargar(t_lector_32_bits *lector)
{
    size_t leido = fread(&lector->buffer, sizeof(uint32_t), 1, lector->archivo);
    if (leido > 0)
    {
        lector->pos = 0;
        lector->bits_restantes = 32;
        return 1; // bloque cargado correctamente
    }
    lector->bits_restantes = 0;
    return 0; // Fin inesperado de archivo
}

int lector_32_bits_leer_bit(t_lector_32_bits *lector)
{
    if (lector->bits_restantes == 0)
    {
        if (!lector_32_bits_cargar(lector))
        {
            return -1; // fin inesperado de archivo
        }
    }
    int bit = (lector->buffer >> (31 - lector->pos)) & 1;
    lector->pos++;
    lector->bits_restantes--;
    return bit;
}
