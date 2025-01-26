#include "helpers.h"

void string_append(char **dest, char *src)
{
    *dest = realloc(*dest, strlen(*dest) + strlen(src) + 1);
    strcat(*dest, src);
}

void string_append_char(char **dest, int c)
{
    char tmp[] = {c, '\0'};
    string_append(dest, tmp);
}

char *drop_n_chars(const char *str, int n)
{
    if (str == NULL)
        return NULL;
    int len = strlen(str);
    if (n >= len)
        return strdup("");
    return strdup(str + n);
}
