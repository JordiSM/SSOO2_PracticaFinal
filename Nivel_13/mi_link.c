/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "directorios.h"

//crea un enlace a un fichero, llamando a la función mi_link() de la capa de directorios

int main(int argc, char **argv) {

    char *nombreDisp, *ruta_fichero_original, *ruta_enlace;
    int mi_link_error;

    //En caso de no haber el número de argumentos correcto
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        return EXIT_FAILURE;
    }

    //Asociamos los parámetros a las varibles correspondientes
    nombreDisp = argv[1];
    ruta_fichero_original = argv[2];
    ruta_enlace = argv[3];

    //Comprobamos que la ruta es válida
    if (ruta_fichero_original[strlen(ruta_fichero_original) - 1] == '/') {
        fprintf(stderr, "Error: %s no es un fichero.\n", ruta_fichero_original);
        return EXIT_FAILURE;
    }

    if (ruta_enlace[strlen(ruta_enlace) - 1] == '/') {
        fprintf(stderr, "Error: %s no es un fichero.\n", ruta_enlace);
        return EXIT_FAILURE;
    }

    //Montamos dispositivo
    if (bmount(nombreDisp) == -1) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }

    //Creamos el enlace de una entrada de directorio 
    if ((mi_link_error = mi_link(ruta_fichero_original, ruta_enlace)) < 0) {
        mostrar_error_buscar_entrada(mi_link_error);
    }

    //Desmontamos dispositivo
    if (bumount() == -1) {
        fprintf(stderr, "Error en bumount\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}