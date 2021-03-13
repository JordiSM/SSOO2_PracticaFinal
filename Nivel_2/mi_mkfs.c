#include "ficheros_basico.h"

/*
  FORMATO DE USO:

 ./mi_mkfs "nombre_de_memoria" "numero_de_bloques" #COMENTARIOS

*/

int main(int argc, char **argv){
    
    if(argc < 3){

        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        perror("Error, número de argumentos no válidos");
        return EXIT_FAILURE;

    }else{

        if(bmount(argv[1])==1){
            exit(EXIT_FAILURE);
        }

        int nbloques = atoi(argv[2]); //conversion a int

        unsigned const char *buff [BLOCKSIZE];
        memset(buff, 0, sizeof(buff)); // iniciar a 0 todos los elementos del array

        for(int i = 0; i < nbloques; i++){
            if((bwrite(i, buff)) == 1){
                exit(EXIT_FAILURE);
            }
        }

        // ---------------------------  NIVEL 2  ------------------------------

        unsigned int ninodos = nbloques/4;

        if(initSB(nbloques, ninodos) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        initMB();
        initAI();




        bumount();
    }
}
