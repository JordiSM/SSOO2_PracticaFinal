
#include "directorios.h"

int main (int argc, char **argv) {
    char *disco, *path;
    int permisos;

    if(argc < 4) {
        fprintf(stderr, "Modo de uso: ./mi_mkdir \"disco\" \"permisos \" \"path\" \n");
        return EXIT_FAILURE;
    }

    disco = argv[1];
    permisos = atoi(argv[2]);
    path = argv[3];

    if(permisos < 0 || permisos > 7){
        fprintf(stderr, "Error, introducir permisos válidos (0 - 7)\n");
        return EXIT_FAILURE;
    }
    if(path[strlen(path) - 1] != '/'){
        fprintf(stderr, "Error: no es un directorio\n");
        fprintf(stderr, "Debe acabar con '/' \n");
        return EXIT_FAILURE;
    }

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
