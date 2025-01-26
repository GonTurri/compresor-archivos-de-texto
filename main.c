#include <stdio.h>
#include "helpers.h"
#include "file_helpers.h"
#include "buffer_bits.h"

// FORMATO DEL ARCHIVO: CANT CARACTERES ARCHIVO ORGINAL - CANT CARACTERES NOMBRE ORIGINAL
// - NOMBRE ORIGINAL - ARBOL HUFFMAN - SECUENCIA COMPRIMIDA

#define CANT_CARACTERES 256

static char *nombre_comprimido;

typedef struct
{
    int caracter;
    int repeticiones;
} t_caracter;

typedef struct nodo
{
    int repeticiones;
    char *caracteres;
    int caracter;
    struct nodo *izq;
    struct nodo *der;
} t_nodo;

int compararRepeticiones(const void *a, const void *b)
{
    t_caracter *caracter_a = (t_caracter *)a;
    t_caracter *caracter_b = (t_caracter *)b;
    return caracter_b->repeticiones - caracter_a->repeticiones;
}

t_nodo *nodo_create_rama(int repeticiones, char *caracteres, t_nodo *izq, t_nodo *der)
{
    t_nodo *nuevo = (t_nodo *)malloc(sizeof(t_nodo));

    if (!nuevo)
    {
        perror("error al asignarle memoria a un nodo");
        exit(EXIT_FAILURE);
    }
    nuevo->repeticiones = repeticiones;
    nuevo->caracteres = caracteres ? strdup(caracteres) : NULL;
    nuevo->izq = izq;
    nuevo->der = der;
    nuevo->caracter = -2;
    return nuevo;
}

t_nodo *nodo_create_hoja(int repeticiones, int caracter)
{
    t_nodo *nuevo = (t_nodo *)malloc(sizeof(t_nodo));
    if (!nuevo)
    {
        perror("error al asignarle memoria a un nodo");
        exit(EXIT_FAILURE);
    }
    nuevo->repeticiones = repeticiones;
    nuevo->caracteres = NULL;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    nuevo->caracter = caracter;
    return nuevo;
}

void nodo_destroy(t_nodo *nodo)
{
    if (!nodo)
        return;

    nodo_destroy(nodo->izq);
    nodo_destroy(nodo->der);

    if (nodo->caracteres)
        free(nodo->caracteres);

    free(nodo);
}

t_nodo *construir_arbol_huffman(t_caracter array_caracteres[], int inicio, int fin, char *cadena_previa, int frecuencia_previa)
{
    if (inicio == fin)
    {
        // estoy en nodo hoja
        return nodo_create_hoja(array_caracteres[inicio].repeticiones, array_caracteres[inicio].caracter);
    }

    int mitad = 0;

    int frecuencia_acumulada = 0;

    char *cadena_caracteres_izq = strdup("");
    for (int i = inicio; i < fin; i++)
    {
        string_append_char(&cadena_caracteres_izq, array_caracteres[i].caracter);
        frecuencia_acumulada += array_caracteres[i].repeticiones;
        if (frecuencia_acumulada >= (frecuencia_previa / 2))
        {
            mitad = i;
            break;
        }
    }

    char *cadena_caracteres_der = drop_n_chars(cadena_previa, strlen(cadena_caracteres_izq));

    t_nodo *izq = construir_arbol_huffman(array_caracteres, inicio, mitad, cadena_caracteres_izq, frecuencia_acumulada);
    t_nodo *der = construir_arbol_huffman(array_caracteres, mitad + 1, fin, cadena_caracteres_der, frecuencia_previa - frecuencia_acumulada);

    free(cadena_caracteres_izq);
    free(cadena_caracteres_der);

    return nodo_create_rama(frecuencia_previa, cadena_previa, izq, der);
}

void imprimirArbol(t_nodo *raiz, int nivel)
{
    if (!raiz)
        return;

    for (int i = 0; i < nivel; i++)
        printf("  "); // Indentación
    if (raiz->der || raiz->izq)
    {
        if (raiz->caracteres != NULL)
        {
            printf("Nodo: '%s', Frecuencia: %d\n", raiz->caracteres, raiz->repeticiones);
        }
        else
            printf("Nodo\n");
    }
    else
    {
        if (raiz->caracter != '\n')
            printf("Hoja: '%c', Frecuencia: %d\n", raiz->caracter, raiz->repeticiones);
        else
            printf("Hoja: '\\n', Frecuencia: %d\n", raiz->repeticiones);
    }

    imprimirArbol(raiz->izq, nivel + 1);
    imprimirArbol(raiz->der, nivel + 1);
}

int buscar_caracter(t_nodo *arbol_huffman, int objetivo, int camino[], int nivel)
{
    // por convencion la izquierda se representa con 0 y la derecha con 1

    if (arbol_huffman == NULL)
        return 0;

    // busco el caracter en una hoja
    if (arbol_huffman->der == NULL && arbol_huffman->izq == NULL && arbol_huffman->caracter == objetivo)
    {
        return 1;
    }

    if (buscar_caracter(arbol_huffman->izq, objetivo, camino, nivel + 1))
    {
        camino[nivel] = 0; // marco izquierda
        return 1;
    }

    if (buscar_caracter(arbol_huffman->der, objetivo, camino, nivel + 1))
    {
        camino[nivel] = 1; // marco derecha
        return 1;
    }

    return 0; // no hallo nada, no deberia pasar
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

void serializar_arbol_huffman(t_nodo *arbol_huffman, FILE *archivo_codificado, t_buffer_32_bits *buffer)
{
    if (arbol_huffman == NULL)
        return;

    if (arbol_huffman->der == NULL && arbol_huffman->izq == NULL)
    {
        buffer_32_escribir_bit(buffer, 1);
        buffer_32_escribir_byte(buffer, arbol_huffman->caracter);
        return;
    }

    buffer_32_escribir_bit(buffer, 0);
    serializar_arbol_huffman(arbol_huffman->izq, archivo_codificado, buffer);
    serializar_arbol_huffman(arbol_huffman->der, archivo_codificado, buffer);
}

t_nodo *deserializar_arbol_huffman(t_lector_32_bits *lector)
{
    // Leer un bit del lector
    int bit = lector_32_bits_leer_bit(lector);
    if (bit == -1)
    {
        perror("error al leer el archivo durante la deserialización del árbol");
        exit(EXIT_FAILURE);
    }

    if (bit == 1)
    {
        // si es una hoja, leer el siguiente carácter (8 bits)
        int prox_caracter = 0;
        for (int i = 0; i < 8; i++)
        {
            int bit_caracter = lector_32_bits_leer_bit(lector);
            if (bit_caracter == -1)
            {
                perror("Error al leer un carácter durante la deserialización del árbol");
                exit(EXIT_FAILURE);
            }
            prox_caracter = (prox_caracter << 1) | bit_caracter;
        }
        return nodo_create_hoja(0, prox_caracter);
    }

    // si no es una hoja, deserializar recursivamente los subárboles izquierdo y derecho
    t_nodo *izq = deserializar_arbol_huffman(lector);
    t_nodo *der = deserializar_arbol_huffman(lector);
    return nodo_create_rama(0, NULL, izq, der);
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

    nombre_comprimido = path_archivo_comprimido;

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

int comparar_arboles(t_nodo *a, t_nodo *b)
{
    if (a == NULL && b == NULL)
        return 1;
    if (a == NULL || b == NULL)
        return 0;
    if (a->caracter != b->caracter)
        return 0;
    return comparar_arboles(a->izq, b->izq) && comparar_arboles(a->der, b->der);
}

void imprimir_camino(int camino[])
{
    for (int i = 0; camino[i] != -1; i++)
    {
        printf("%d - ", camino[i]);
    }
    printf("\n");
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
