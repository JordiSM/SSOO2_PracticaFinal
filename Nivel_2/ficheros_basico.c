#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){
    
    int tamMB = (nbloques/8)/BLOCKSIZE;

    if ((nbloques/8)%BLOCKSIZE != 0){
        tamMB ++;
    }

    return tamMB;
}

int tamAI(unsigned int ninodos){

    int tamAI = (ninodos * INODOSIZE) / BLOCKSIZE;

    if ((ninodos * INODOSIZE) % BLOCKSIZE != 0){
        tamAI++;
    }

    return tamAI;
}

int initSB(unsigned int nbloques, unsigned int ninodos){   

    struct superbloque SB;

    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    return bwrite(posSB, &SB);
}

int initMB(){
    
    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, 0, sizeof(bufferMB));

    struct superbloque SB;

    if (bread(posSB, &SB) == 1){
        //Error a la hora de leer la posición del SuperBloque en el Fichero
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

/*  Bits a 1 en caso de sobrar en el bloque


    int metadatos = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);
    int completar = metadatos % 8; 
    int cantidad_bytes = metadatos/8;
    cantidad_bytes = cantidad_bytes / 255;

    if(completar != 0){
        int n = 0;
        for (int i = 7; i >= (8 - completar); i--){
            n = n + pow(2,i);
        }
    }
*/

    //int sizeMB = tamMB(SB.totBloques);
    int last = SB.posUltimoBloqueMB;
    for(int i = SB.posPrimerBloqueMB; i < last; i++){
        if(bwrite(i,&bufferMB)== 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
    }
    
    return 0;
}

int initAI(){

    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    struct superbloque SB;
    int contInodos, i, j;

    if (bread(posSB, &SB) == 1){
        //Error a la hora de leer la posición del SuperBloque en el Fichero
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Primera posición del primer Inodo libre
    contInodos = SB.posPrimerInodoLibre + 1;

    //Recorremos desde el primer bloque hasta el último
    for(i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){

        //En cada bloque hay BLOCKSIZE/INODOSIZE = 8 inodos
        for(j = 0; j < BLOCKSIZE/INODOSIZE; j++){
            inodos[j].tipo = "l";   //Señalamos tipo "l" ---> libre

            //Si hay nodos libres aún inicializamos el primer elemento con la posición
            //que ocupa en la liste. Si es el último apuntará a UINT_MAX
            if(contInodos < SB.totInodos){
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }else{
                inodos[j].punterosDirectos[0] = UINT_MAX;
            }
        }

        //Escribimos
        if(bwrite(i,&inodos) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    return 0;
}