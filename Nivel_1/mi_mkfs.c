#include "bloques.h"

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
        memset(buff, 1, sizeof(buff)); // iniciar a 0 todos los elementos del array

        for(int i = 0; i < nbloques; i++){
            if((bwrite(i, buff)) == 1){
                exit(EXIT_FAILURE);
            }
        }
        
        bumount();
    }
}