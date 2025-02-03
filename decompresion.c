#include "decompresion.h"

void descomprimir_archivo(const char *path_archivo)
{
    FILE *f = fopen(path_archivo, "rb");
    int cant_caracteres, largo_nombre_original;
    char *nombre_original;
    t_nodo *arbol_huffman;
    if (fread(&cant_caracteres, sizeof(int), 1, f) != 1)
    {
        perror("error en el proceso de decompresión");
        exit(EXIT_FAILURE);
    }

    if (fread(&largo_nombre_original, sizeof(int), 1, f) != 1)
    {
        perror("error en el proceso de decompresión");
        exit(EXIT_FAILURE);
    }
    nombre_original = calloc(largo_nombre_original + 1, sizeof(char));
    if (fread(nombre_original, sizeof(char), largo_nombre_original, f) != largo_nombre_original)
    {
        perror("error en el proceso de decompresión");
        exit(EXIT_FAILURE);
    }

    t_lector_32_bits *lector = lector_32_bits_new(f);

    arbol_huffman = deserializar_arbol_huffman(lector);

    if (!arbol_huffman)
    {
        perror("error en el proceso de decompresión");
        exit(EXIT_FAILURE);
    }
    free(lector);

    lector = lector_32_bits_new(f);

    char *nombre_descomprimido = generar_nuevo_nombre_archivo(nombre_original);

    FILE *archivo_descomprimido = fopen(nombre_descomprimido, "w+");
    if (!archivo_descomprimido)
    {
        perror("error al abrir el archivo para descomprimir");
        exit(EXIT_FAILURE);
    }

    t_nodo *temp = arbol_huffman;
    int bit;
    while (cant_caracteres)
    {
        // encuentro una hoja
        while (temp->der || temp->izq)
        {
            bit = lector_32_bits_leer_bit(lector);
            if (bit == -1)
            {
                perror("error al intentar descomprimir el archivo");
                exit(EXIT_FAILURE);
            }
            temp = bit ? temp->der : temp->izq;
        }
        fputc(temp->caracter, archivo_descomprimido);
        temp = arbol_huffman;
        cant_caracteres--;
    }
    fclose(f);
    fclose(archivo_descomprimido);
    free(lector);
    free(nombre_original);
    free(nombre_descomprimido);
    nodo_destroy(arbol_huffman);
}

