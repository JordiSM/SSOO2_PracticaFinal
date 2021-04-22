#include "ficheros.h"

int main(int argc, char **argv){

    // Verificamos si el número de argumentos introducido es válido
    if(argc != 4){
        fprintf(stderr, "Error, número de argumentos no válidos\n");
        return EXIT_FAILURE;
    }

    // Montamos el dispositivo con el nombre que se ha pasado por parámetro
    if(bmount(argv[1]) == 1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return EXIT_FAILURE;
    }

    // El número de bytes corresponde al argumento 4 que nos pasan por consola
    int nbytes = atoi( argv[3] ); 

    // El número de inodo corresponde al argumento número 3 que nos pasan por consola
    int ninodo = atoi( argv[2] ) ;

    if(nbytes == 0){
        // Si el inodo se encuentra vacío, únicamente liberamos el inodo
        if(liberar_inodo(ninodo) == 1){
            fprintf(stderr, "Error en liberar_inodo\n");
            exit(EXIT_FAILURE);
        }
    }else{
        // Sino, truncamos la cantidad de bytes que se ha pasado por parámetro
        if(mi_truncar_f(ninodo,nbytes) == 1){
            fprintf(stderr, "Error en mi_truncar_f\n");
            exit(EXIT_FAILURE);
        }
    }

    // Lectura del estado del inodo para verificar que todo está en orden
    struct STAT stat;
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    
    mi_stat_f(ninodo,&stat);
    // Comprobamos los datos del inodo
    printf("DATOS DEL INODO\n");

    //Mostramos por pantalla
    printf("tipo=%c\n",stat.tipo);
    printf("permisos=%d\n",stat.permisos);

    ts = localtime(&stat.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&stat.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&stat.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

	fprintf(stdout,"atime: %s \nctime: %s \nmtime: %s\n",atime,ctime,mtime);

    // Comprobamos el estado del tamaño en bytes
    printf("tamEnBytesLog=%d\n",stat.tamEnBytesLog);
    // Comprobamos el estado de los bloques ocupados
    printf("numBloquesOcupados=%d\n",stat.numBloquesOcupados);

    //Y finalmente desmontamos el dispositivo
    if(bumount() == 1){
        fprintf(stderr,"Error al desmontar.\n");
        return EXIT_FAILURE;
    }

}