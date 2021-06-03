/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "directorios.h"
//#include <string.h>

int main(int argc, char **argv) {

    char *nombreDisp, *ruta;
    int offset, nbytes, mi_write_error;

    //En caso de que haya más o menos parámetros de lo correspondido
    if (argc != 5) {
        fprintf(stderr, "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return EXIT_FAILURE;
    }

    //Asociamos los párametros a cada variable
    nombreDisp = argv[1];
    ruta = argv[2];
    nbytes = strlen(argv[3]);
    offset = atoi(argv[4]);

    printf("Longitud de texto: %d \n", nbytes);

    //Comprobamos si es un fichero 
    if (ruta[strlen(ruta) - 1] == '/') {
        fprintf(stderr, "Error: %s se trata de un directorio.\n", ruta);
        return EXIT_FAILURE;
    }

    //Montamos el dispositivo
    if (bmount(nombreDisp) == -1) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }

    //Comprobamos que tiene permisos de escritura
    if ((mi_write_error = mi_write(ruta, argv[3], offset, nbytes)) < 0) {
        mostrar_error_buscar_entrada(mi_write_error);
        printf("Bytes escritos: 0\n");
    } else { //Mostramos la cantidad de bytes
        printf("Bytes escritos: %d\n", mi_write_error);
    }

    //Desmontamos el dispositivo
    if (bumount() == -1) {
        fprintf(stderr, "Error en bumount\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}