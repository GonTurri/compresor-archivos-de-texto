#ifndef COMPRESION_H_INCLUDED
#define COMPRESION_H_INCLUDED

#include "arbol_huffman.h"
#include "buffer_bits.h"
#include "file_helpers.h"

#define CANT_CARACTERES 256

void comprimir_archivo(const char *path_archivo);


#endif 

