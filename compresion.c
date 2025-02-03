#include "compresion.h"

static int compararRepeticiones(const void *a, const void *b)
{
    t_caracter *caracter_a = (t_caracter *)a;
    t_caracter *caracter_b = (t_caracter *)b;
    return caracter_b->repeticiones - caracter_a->repeticiones;
}

static void codificar_archivo(const char *nombre_archivo, t_nodo *arbol_huffman, int camino[], int cant_caracteres_archivo, t_buffer_32_bits *buffer)
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

