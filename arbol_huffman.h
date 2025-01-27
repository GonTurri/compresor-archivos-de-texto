#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "buffer_bits.h"
#include "helpers.h"

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

t_nodo *nodo_create_rama(int repeticiones, char *caracteres, t_nodo *izq, t_nodo *der);


t_nodo *nodo_create_hoja(int repeticiones, int caracter);

void nodo_destroy(t_nodo *nodo);

t_nodo *construir_arbol_huffman(t_caracter array_caracteres[], int inicio, int fin, char *cadena_previa, int frecuencia_previa);

void imprimirArbol(t_nodo *raiz, int nivel);

int buscar_caracter(t_nodo *arbol_huffman, int objetivo, int camino[], int nivel);

void serializar_arbol_huffman(t_nodo *arbol_huffman, FILE *archivo_codificado, t_buffer_32_bits *buffer);

t_nodo *deserializar_arbol_huffman(t_lector_32_bits *lector);

int comparar_arboles(t_nodo *a, t_nodo *b);

void imprimir_camino(int camino[]);

