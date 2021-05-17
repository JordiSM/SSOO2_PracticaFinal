#include "directorios.h"
#include <string.h>

int main(int argc, char **argv) {
    struct STAT p_stat;
    char *nombre_dispositivo, *ruta;
    int mi_stat_err;

    if (argc != 3) {
        fprintf(stderr, "Formato incorrecto: ./mi_stat <disco> </ruta>\n");
        return EXIT_FAILURE;
    }

    nombre_dispositivo = argv[1];
    ruta = argv[2];

    if (bmount(nombre_dispositivo) == 1) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }

    if ((mi_stat_err = mi_stat(ruta, &p_stat)) < 0) {
        mostrar_error_buscar_entrada(mi_stat_err);
    } else {
        printf("tipo: %c\n", p_stat.tipo);
        printf("permisos: %d\n", p_stat.permisos);
        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];
        ts = localtime(&p_stat.atime);
	    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	    ts = localtime(&p_stat.mtime);
	    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	    ts = localtime(&p_stat.ctime);
	    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf("nlinks: %d\n", p_stat.nlinks);
        printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
        printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);
    }
    
    if (bumount() == 1) {
        fprintf(stderr, "Error en bumount\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}