#include "helpers.h"
#include "file_helpers.h"
#include "buffer_bits.h"
#include "arbol_huffman.h"

// FORMATO DEL ARCHIVO: CANT CARACTERES ARCHIVO ORIGINAL - CANT CARACTERES NOMBRE ORIGINAL
// - NOMBRE ORIGINAL - ARBOL HUFFMAN - SECUENCIA COMPRIMIDA

#define CANT_CARACTERES 256

int compararRepeticiones(const void *a, const void *b)
{
    t_caracter *caracter_a = (t_caracter *)a;
    t_caracter *caracter_b = (t_caracter *)b;
    return caracter_b->repeticiones - caracter_a->repeticiones;
}

void codificar_archivo(const char *nombre_archivo, t_nodo *arbol_huffman, int camino[], int cant_caracteres_archivo, t_buffer_32_bits *buffer)
{
    FILE *archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL)
    {
        perror("error al abrir el archivo original");
        exit(EXIT_FAILURE);
    }
    if (!buffer)
    {
        perror("error al pedir memoria");
        exit(EXIT_FAILURE);
    }

    uint32_t prox_caracter;
    for (int i = 0; i < cant_caracteres_archivo; i++)
    {

        prox_caracter = fgetc(archivo);
        buscar_caracter(arbol_huffman, prox_caracter, camino, 0);
        for (int j = 0; camino[j] != -1; j++)
        {
            buffer_32_escribir_bit(buffer, camino[j]);
        }
        // resetear camino
        memset(camino, -1, CANT_CARACTERES * sizeof(int));
    }
    vaciar_buffer(buffer);
    fclose(archivo);
}

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

void comprimir_archivo(const char *path_archivo)
{

    t_caracter caracteres[CANT_CARACTERES];
    int camino_caracter[CANT_CARACTERES];

    for (int i = 0; i < CANT_CARACTERES; i++)
    {
        caracteres[i].caracter = i;
        caracteres[i].repeticiones = 0;
        camino_caracter[i] = -1;
    }
    int caracteres_archivo = 0;

    FILE *f = fopen(path_archivo, "r");
    if (f == NULL)
    {
        perror("Error al intentar comprimir el archivo\n");
        exit(EXIT_FAILURE);
    }

    for (int b; (b = fgetc(f)) != EOF; caracteres[b].repeticiones++)
        caracteres_archivo++;

    fclose(f);

    qsort(caracteres, CANT_CARACTERES, sizeof(t_caracter), compararRepeticiones);

    // encuentro hasta el ultimo caracter que aparezca

    char *cadena_caracteres_total = strdup("");
    int final = 0;
    while (final < CANT_CARACTERES && caracteres[final].repeticiones)
    {
        string_append_char(&cadena_caracteres_total, caracteres[final].caracter);
        final++;
    }
    t_nodo *arbol_huffman = construir_arbol_huffman(caracteres, 0, final - 1, cadena_caracteres_total, caracteres_archivo);

    free(cadena_caracteres_total);

    // imprimirArbol(arbol_huffman, 0);

    char *path_archivo_comprimido = get_path_archivo_comprimido(path_archivo);

    if (path_archivo_comprimido == NULL)
    {
        perror("error al generar el path para el archivo comprimido");
        exit(EXIT_FAILURE);
    }

    FILE *archivo_comprimido = fopen(path_archivo_comprimido, "wb+");
    fwrite(&caracteres_archivo, sizeof(int), 1, archivo_comprimido);
    int largo_nombre_original = strlen(path_archivo);
    fwrite(&largo_nombre_original, sizeof(int), 1, archivo_comprimido);
    fwrite(path_archivo, sizeof(char), largo_nombre_original, archivo_comprimido);

    t_buffer_32_bits *buf = buffer__32_bits_new(archivo_comprimido);
    serializar_arbol_huffman(arbol_huffman, archivo_comprimido, buf);
    vaciar_buffer(buf);
    free(buf);

    buf = buffer__32_bits_new(archivo_comprimido);
    codificar_archivo(path_archivo, arbol_huffman, camino_caracter, caracteres_archivo, buf);

    fclose(archivo_comprimido);
    nodo_destroy(arbol_huffman);
    free(buf);
}

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
