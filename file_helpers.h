#ifndef FILE_HELPERS_H_INCLUDED
#define FILE_HELPERS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXTENSION ".comprimido"

int existe_archivo(const char *path_archivo);

char *generar_nuevo_nombre_archivo(const char *path_orginal);

// devuelve NULL en caso de error
char *get_path_archivo_comprimido(const char *path_archivo);

#endif
