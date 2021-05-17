#include "directorios.h"

int main(int argc, char **argv) {
    char *nombre_dispositivo, *ruta;
    int permisos, mi_chmod_err;

    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n");
        return EXIT_FAILURE;
    }

    nombre_dispositivo = argv[1];
    permisos = atoi(argv[2]);

    ruta = argv[3];
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, "Error: Permisos ha de estar entre 0 y 7.\n");
        return EXIT_FAILURE;
    }

    if (bmount(nombre_dispositivo) == 1) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }

    if ((mi_chmod_err = mi_chmod(ruta, permisos)) < 0) {
        mostrar_error_buscar_entrada(mi_chmod_err);
    }

    if (bumount() == 1) {
        fprintf(stderr, "Error en bumount\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}