cflags = -g -Wall -pedantic-errors -D_GNU_SOURCE -std=c2x

all : buffer_bits.o file_helpers.o helpers.o arbol_huffman.o compresion.o decompresion.o main.o 
	gcc buffer_bits.o helpers.o file_helpers.o arbol_huffman.o compresion.o decompresion.o main.o  -o compresor

main.o: main.c buffer_bits.h helpers.h file_helpers.h arbol_huffman.h
	gcc $(cflags) -c main.c 

helpers.o: helpers.c helpers.h
	gcc $(cflags) -c helpers.c

arbol_huffman.o: arbol_huffman.c arbol_huffman.h buffer_bits.h helpers.h
	gcc $(cflags) -c arbol_huffman.c

buffer_bits.o: buffer_bits.c buffer_bits.h
	gcc $(cflags) -c buffer_bits.c

file_helpers.o: file_helpers.c file_helpers.h
	gcc $(cflags) -c file_helpers.c

compresion.o: compresion.c file_helpers.h arbol_huffman.o buffer_bits.o
	gcc $(cflags) -c compresion.c

decompresion.o: decompresion.c file_helpers.h arbol_huffman.o buffer_bits.o
	gcc $(cflags) -c decompresion.c


clean: 
	rm buffer_bits.o helpers.o file_helpers.o arbol_huffman.o compresion.o decompresion.o  main.o compresor