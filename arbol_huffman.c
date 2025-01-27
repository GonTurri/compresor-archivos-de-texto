#include "arbol_huffman.h"

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


