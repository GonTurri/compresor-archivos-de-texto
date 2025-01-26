# Compresor de archivos de texto

Compresor casero para comprimir archivos de texto que implementa el [algoritmo de huffman](https://en.wikipedia.org/wiki/Huffman_coding)

## Consideraciones
- Este compresor esta pensado para comprimir archivos tipo txt solo con caracteres de ascii extendido (codigo ascii de 0 a 255)
- La compresión que logra este programa no es superior a ningún compresor profesional como por ejemplo zip
- El prinicipal objetivo de este proyecto es aplicar conocimientos teóricos

## Guía de uso

### Compilar el proyecto
``` make all ```

### Comprimir un archivo
``` ./compresor ejemplo.txt ```

Esto generará un archivo **ejemplo.comprimido**

### Descomprimir un archivo
El programa intentará descomprimir cualquier archivo con la extesión **.comprimido**

``` ./compresor ejemplo.comprimido ```

### Para borrar ejecutable y objetos
``` make clean ```
