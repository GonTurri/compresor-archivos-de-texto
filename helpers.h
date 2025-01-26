#ifndef STRING_HELPERS_H_INCLUDED
#define STRING_HELPERS_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void string_append(char **dest, char *src);

void string_append_char(char **dest, int c);

char *drop_n_chars(const char *str, int n);

#endif
