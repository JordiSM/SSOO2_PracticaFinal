/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "directorios.h"
#include <string.h>

int main(int argc, char **argv) {
    struct STAT p_stat;
    char *nombre_dispositivo, *ruta;
    int mi_stat_err;

    //Comprobamos numero de argumentos
    if (argc != 3) {
        fprintf(stderr, "Formato incorrecto: ./mi_stat <disco> </ruta>\n");
        return EXIT_FAILURE;
    }

    nombre_dispositivo = argv[1];
    ruta = argv[2];

    //Montamos dispositivo
    if (bmount(nombre_dispositivo) == 1) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }

    //Llamamos a la función mi_stat
    if ((mi_stat_err = mi_stat(ruta, &p_stat)) < 0) {
        mostrar_error_buscar_entrada(mi_stat_err);
    } else {

        //Imprimimos todos los datos necesarios
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

        fprintf(stdout,"atime: %s \nctime: %s \nmtime: %s\n",atime,ctime,mtime);

        printf("nlinks: %d\n", p_stat.nlinks);
        printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
        printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);
    }
    
    //Desmontamos dispositivo
    if (bumount() == 1) {
        fprintf(stderr, "Error en bumount\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}