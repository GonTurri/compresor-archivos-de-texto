cflags = -g -Wall -pedantic-errors -D_GNU_SOURCE -std=c2x

all : buffer_bits.o file_helpers.o helpers.o main.o 
	gcc buffer_bits.o helpers.o file_helpers.o main.o  -o compresor

main.o: main.c buffer_bits.h helpers.h file_helpers.h 
	gcc $(cflags) -c main.c 

helpers.o: helpers.c helpers.h
	gcc $(cflags) -c helpers.c

buffer_bits.o: buffer_bits.c buffer_bits.h
	gcc $(cflags) -c buffer_bits.c

file_helpers.o: file_helpers.c file_helpers.h
	gcc $(cflags) -c file_helpers.c


clean: 
	rm buffer_bits.o helpers.o file_helpers.o main.o compresor