/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "directorios.h"

int main (int argc, char **argv) {
    char *disco, *path;
    int permisos;

    if(argc < 4) {
        fprintf(stderr, "Sintaxis: ./mi_mkdir <disco> <permisos> </ruta> \n");
        return EXIT_FAILURE;
    }

    disco = argv[1];
    permisos = atoi(argv[2]);
    path = argv[3];

    if(permisos < 0 || permisos > 7){
        fprintf(stderr, "Error: modo inválido: <<9>> \n");
        return EXIT_FAILURE;
    }
    
    /*if(path[strlen(path) - 1] != '/'){
        fprintf(stderr, "Error: no es un directorio\n");
        fprintf(stderr, "Debe acabar con '/' \n");
        return EXIT_FAILURE;
    }*/

    if(bmount(disco) == EXIT_FAILURE){
        fprintf(stderr, "Error al montar el dispositivo");
        return EXIT_FAILURE;
    }

    int estado = mi_creat(path, permisos);

    if(estado < 0){
        
        mostrar_error_buscar_entrada(estado);
        
        if(bumount() == EXIT_FAILURE){
            fprintf(stderr, "Error al desmontar el dispositivo \n");
            return EXIT_FAILURE;
        }

        return EXIT_FAILURE;
    }

    if(bumount() == EXIT_FAILURE){
        fprintf(stderr, "Error al desmontar el dispositivo \n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
