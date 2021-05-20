#include "directorios.h"

int main(int argc, char **argv) {

    char *nombreDisp, *ruta;
    char buffer[2000];
    int mi_dir_error;

    //En caso de que no haya 3 argumentos mostramos el error por pantalla
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
        return EXIT_FAILURE;
    }

    //El segundo argumento que nos pasan por parámetro desde la terminal es el nombre del dispositivo, y lo guardamos
    nombreDisp = argv[1];
    //El tercer argumento que nos pasan por parámetro desde la terminal es la ruta del directorio, y lo guardamos
    ruta = argv[2];

    //Montamos el dispositivo
    if (bmount(nombreDisp) == EXIT_FAILURE) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }
    
    //En caso de que la función mi_dir sea menor que cero mostramos el error.
    if ((mi_dir_error = mi_dir(ruta, buffer)) < 0) {
        mostrar_error_buscar_entrada(mi_dir_error);
    } else { //Sino imprimimos el total de entradas

        printf("Total: %d\n", mi_dir_error);

        if (mi_dir_error > 0) { //y visualizamos los campos correspondientes
            printf("\033[1;34mTipo\t\tModo\tmTime\t\t\t\tTamaño\t\tNombre\033[0m\n");
            printf("%s", buffer);
        }
    }

    //Desmontamos el dispositivo
    if (bumount() == EXIT_FAILURE) {
        fprintf(stderr, "Error en bumount\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}