/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "ficheros.h"

int main(int argc, char **argv){

    if(argc < 4){
        fprintf(stderr,"Uso: permitir \"nombre_dispositivo\" \"ninodo\" \"permisos\"\n");
        exit(-1);
    }
    //Montamos el dispositivo
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en bmount\n");
        exit(-1);
    }
    
    int ninodo = atoi(argv[2]);
    int permiso = atoi(argv[3]);
    
    //Intentamos cambiar los permisos del inodo pasado por parametro
    if(mi_chmod_f(ninodo,permiso) == -1){
        fprintf(stderr,"Error en mi_chmod_f \n");
        exit(-1);
    };

    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        exit(-1);
    }
}