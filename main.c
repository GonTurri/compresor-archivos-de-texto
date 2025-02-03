#include "file_helpers.h"
#include "compresion.h"
#include "decompresion.h"

// FORMATO DEL ARCHIVO: CANT CARACTERES ARCHIVO ORIGINAL - CANT CARACTERES NOMBRE ORIGINAL
// - NOMBRE ORIGINAL - ARBOL HUFFMAN - SECUENCIA COMPRIMIDA

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Se esperaba: %s <nombre_del_archivo>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *nombre_archivo = argv[1];
    char *extension = strrchr(nombre_archivo, '.');

    if (!existe_archivo(nombre_archivo))
    {
        perror("no se encuentra el archivo solicitado");
        return EXIT_FAILURE;
    }
    if (extension && !strcmp(extension, EXTENSION))
    {
        printf("descomprimiendo archivo %s...\n", nombre_archivo);
        descomprimir_archivo(nombre_archivo);
        printf("Se ha descomprimido %s con exito!\n", nombre_archivo);
        return EXIT_SUCCESS;
    }

    printf("comprimiendo archivo %s...\n", nombre_archivo);
    comprimir_archivo(nombre_archivo);
    printf("se ha comprimido %s con exito!\n", nombre_archivo);

    return EXIT_SUCCESS;
}
