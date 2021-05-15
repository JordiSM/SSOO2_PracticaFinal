#include "directorios.h"

int main(int argc, char *argv[]){
    
    char *ruta, *nomDispositivo;

    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_rm disco /ruta\n");
        return -1;
    }

    nomDispositivo = argv[1];
    ruta = argv[2];

    if (bmount(nomDispositivo) == -1) {
        fprintf(stderr, "Error en bmount en mi_rm.c \n");
        return -1;
    }

    if(mi_unlink(ruta) == -1){
        fprintf(stderr, "Error en mi_unlink en mi_rm.c \n");
        return -1;
    }

    if (bumount() == -1) {
        fprintf(stderr, "Error: error al cerrar el fichero.\n");
        return -1;
    }
    
    return 0;
}