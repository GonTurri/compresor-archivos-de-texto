#include "file_helpers.h"

int existe_archivo(const char* path_archivo){
    FILE* f = fopen(path_archivo,"r");
    if(f){
        fclose(f);
        return 1;
    }
    return 0;
}

char* generar_nuevo_nombre_archivo(const char* path_original){

    if(!existe_archivo(path_original))
        return strdup(path_original);

    int cont = 1;
    const char* extension = strrchr(path_original, '.'); 
    int base_len = extension ? (extension - path_original) : strlen(path_original);
    size_t posible_tamanio = strlen(path_original) + 10;
    char* nombre = calloc(posible_tamanio,sizeof(char));
    if(nombre == NULL)
        return NULL;

        do {
            if(extension){
                snprintf(nombre,posible_tamanio,"%.*s_%d%s",base_len,path_original,cont++,extension);
            }
            else{
                snprintf(nombre,posible_tamanio,"%s_%d",path_original,cont++);
            }
        } while(existe_archivo(nombre));

    return nombre;

}

char* get_path_archivo_comprimido(const char* path_archivo){
    char* path_comprimido;
    size_t len_nueva_extension = strlen(EXTENSION);
    size_t len_con_nueva_extension;
    char* ultimo_punto = strrchr(path_archivo,'.');
    char* ultima_barra = strrchr(path_archivo,'/');

    // verficar si tiene una extension valida

    if(ultimo_punto && (!ultima_barra || (ultimo_punto > ultima_barra))){
        // resta de punteros, pegriloso
        size_t len_sin_extension = ultimo_punto - path_archivo;
        len_con_nueva_extension = len_sin_extension + len_nueva_extension;
        path_comprimido = calloc(len_con_nueva_extension +1,1);
        if(path_comprimido == NULL)
            return NULL;
        strncpy(path_comprimido,path_archivo,len_sin_extension);
        strcat(path_comprimido,EXTENSION);
        return path_comprimido;
    }

    // no tiene extension 
    len_con_nueva_extension = strlen(path_archivo) + len_nueva_extension;
    path_comprimido = calloc(len_con_nueva_extension+1,1); 
    if(path_comprimido == NULL) return NULL;
    strcpy(path_comprimido,path_archivo);
    strcat(path_comprimido,EXTENSION);
    return path_comprimido;
}

