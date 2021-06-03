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

    //Comprueba los argumentos que sean correctos
    if(argc < 4) {
        fprintf(stderr, "Sintaxis: ./mi_mkdir <disco> <permisos> </ruta> \n");
        return EXIT_FAILURE;
    }

    disco = argv[1];
    permisos = atoi(argv[2]);
    path = argv[3];

    //Comprueba que los permisos sea un número válido
    if(permisos < 0 || permisos > 7){
        fprintf(stderr, "Error: modo inválido: <<9>> \n");
        return EXIT_FAILURE;
    }

    if(bmount(disco) == EXIT_FAILURE){
        fprintf(stderr, "Error al montar el dispositivo");
        return EXIT_FAILURE;
    }

    //Crea el directorio
    int estado = mi_creat(path, permisos);

    //Comprueba si hay algún error buscando la entrada
    if(estado < 0){
        
        //Muestra el error de buscar entrada
        mostrar_error_buscar_entrada(estado);
        
        //Desmonta dispositivo
        if(bumount() == EXIT_FAILURE){
            fprintf(stderr, "Error al desmontar el dispositivo \n");
            return EXIT_FAILURE;
        }

        return EXIT_FAILURE;
    }

    //Desmonta dispositivo
    if(bumount() == EXIT_FAILURE){
        fprintf(stderr, "Error al desmontar el dispositivo \n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
