#include "ficheros.h"

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){

    struct inodo inodo;

    //lectura del inodo que nos pasan por parámetro
    if(leer_inodo(ninodo,&inodo) == 1){
        fprintf(stderr,"Error de lectura.\n");
        return EXIT_FAILURE;
    }

    //Si el inodo tiene permisos de lectura
    if((inodo.permisos & 2) == 2){
        //En caso de sobrepasar el tamaño del fichero en bytes lógicos no se puede truncar
        if(inodo.tamEnBytesLog < nbytes){
            fprintf(stderr,"Error. No se puede truncar.\n");
            return EXIT_FAILURE;
        }

        int primerBL;

        if(nbytes % BLOCKSIZE == 0){
            primerBL =  nbytes / BLOCKSIZE;            
        }else{
            primerBL =  (nbytes / BLOCKSIZE) + 1;
        }

        //Llamamos a la función liberar_bloques_inodo para liberar los bloques relacionados con el inodo
        //desde el primer bloque calculado anteriormente
        int bloquesLiberados = liberar_bloques_inodo(primerBL,&inodo); 

        // Actualización de los datos del inodo
        inodo.mtime = time(NULL); 
        inodo.ctime = time(NULL); 

        //truncar inodo
        inodo.tamEnBytesLog = nbytes; 

        //resta de los bloques liberados
        inodo.numBloquesOcupados -= bloquesLiberados; 

        //Escritura del inodo actualizado
        if( escribir_inodo(ninodo,inodo) == 1){
            fprintf(stderr,"Error de escritura.\n");
            return EXIT_FAILURE;
        }
        // Devolución de la cantidad de bloques liberados
        fprintf(stdout, "Bloques liberados: %d\n", bloquesLiberados);

        return bloquesLiberados;
    } else {
        fprintf(stderr, "Error de permisos de lectura\n");
        return EXIT_FAILURE;
    }
}