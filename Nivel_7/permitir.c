/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "ficheros_basico.h"

int main(int argc, char **argv){

    if(argc < 4){
        fprintf(stderr,"Uso: permitir \"nombre_dispositivo\" \"ninodo\" \"permisos\"\n");
        exit(EXIT_FAILURE);
    }
    //Montamos el dispositivo
    if(bmount(argv[1]) == EXIT_FAILURE){
        fprintf(stderr,"Error en bmount\n");
        exit(EXIT_FAILURE);
    }
    
    int ninodo = atoi(argv[2]);
    int permiso = atoi(argv[3]);
    
    //Intentamos cambiar los permisos del inodo pasado por parametro
    if(mi_chmod_f(ninodo,permiso) == EXIT_FAILURE){
        fprintf(stderr,"Error en mi_chmod_f \n");
        exit(EXIT_FAILURE);
    };

    // Desmontamos el dispositivo
    if(bumount() == EXIT_FAILURE){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        exit(EXIT_FAILURE);
    }
}