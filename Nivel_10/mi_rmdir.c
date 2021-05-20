#include "directorios.h"

int main(int argc, char *argv[]){
    
    char *ruta, *nomDispositivo;
    int error;

    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_rmdir disco /ruta\n");
        return -1;
    }

    nomDispositivo = argv[1];
    ruta = argv[2];

    if(ruta[strlen(ruta) - 1] != '/') {
        fprintf(stderr, "Error: %s es un fichero.\n", ruta);
        return -1;
    }
    
    if (bmount(nomDispositivo) == -1) {
        fprintf(stderr, "Error en bmount en mi_rmdir.c \n");
        return -1;
    }

    if((error = mi_unlink(ruta)) < 0){
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    if (bumount() == -1) {
        fprintf(stderr, "Error: error al cerrar el fichero.\n");
        return -1;
    }
    
    return 0;
}