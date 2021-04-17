#include "ficheros_basico.h"

int main(int argc, char **argv){
    // Validamos si la sintaxis es correcta
    if(argc != 4){
        fprintf(stderr,"Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return -1;
    }

    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // Creamos estas variables para mejorar la legibilidad
    int ninodo = atoi( argv[2] );
    int permiso = atoi ( argv[3] );
    // Cambiamos los permisos del inodo pasado por parámetro
    mi_chmod_f(ninodo,permiso);
    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }
}