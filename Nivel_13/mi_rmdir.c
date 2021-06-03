/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "directorios.h"

int main(int argc, char *argv[]){
    
    char *ruta, *nomDispositivo;
    int error;

    //Comprobamos el número de argumentos
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_rmdir disco /ruta\n");
        return -1;
    }

    nomDispositivo = argv[1];
    ruta = argv[2];

    //Comprobamos en que sea un fichero. En caso de ser un directorio devolverá un error.
    if(ruta[strlen(ruta) - 1] != '/') {
        fprintf(stderr, "Error: %s es un fichero.\n", ruta);
        return -1;
    }
    
    //Montamos el dispositivo
    if (bmount(nomDispositivo) == -1) {
        fprintf(stderr, "Error en bmount en mi_rmdir.c \n");
        return -1;
    }

    //Borra el fichero
    if((error = mi_unlink(ruta)) < 0){
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    //Desmonta el dispositivo
    if (bumount() == -1) {
        fprintf(stderr, "Error: error al cerrar el fichero.\n");
        return -1;
    }
    
    return 0;
}