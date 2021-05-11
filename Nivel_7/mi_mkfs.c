/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi

FORMATO DE USO:

 ./mi_mkfs "nombre_de_memoria" "numero_de_bloques" #COMENTARIOS
    
*/

#include "ficheros_basico.h"

/*

*/

int main(int argc, char **argv){
    
    if(argc < 3){
        printf("Error, número de argumentos no válidos\n");
        printf("Formato de uso :\n\t./mi_mkfs \"nombre_de_memoria\" \"numero_de_bloques\" #COMENTARIOS\n");
        fflush(stdout);
        return -1;

    }else{

        if(bmount(argv[1])==-1){
            exit(-1);
        }

        int nbloques = atoi(argv[2]); //conversion a int

        unsigned const char *buff [BLOCKSIZE];
        memset(buff, 0, sizeof(buff)); // iniciar a 0 todos los elementos del array

        for(int i = 0; i < nbloques; i++){
            if((bwrite(i, buff)) == -1){
                exit(-1);
            }
        }

        // ---------------------------  NIVEL 2  ------------------------------

        unsigned int ninodos = nbloques/4;
        
        if(initSB(nbloques, ninodos) == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            printf("Error en la función initSB()");
            exit(-1);
        }
        
        if(initMB() == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            printf("Error en la función initMB()");
            exit(-1);
        }
        if(initAI() == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            printf("Error en la función initAI()");
            exit(-1);
        }

        // ---------------------------  NIVEL 3  ------------------------------
        
        struct superbloque SB;
        
        if (bread(posSB, &SB) == -1){
            //Error a la hora de leer la posición del SuperBloque en el Fichero
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return -1;
        }

        for(int i = posSB; i < SB.posPrimerBloqueDatos; i++){
            if(escribir_bit(i,1) == -1){
                return -1;
            }
        }


        if(reservar_inodo('d', 7) == -1) { // Aquí se pone -1, debido a que puede 
                                           // resultar el caso de reservar el inodo 1
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            printf("Error en la función initMB()");
            exit(-1);
        }
        
        bumount();
    }
}
