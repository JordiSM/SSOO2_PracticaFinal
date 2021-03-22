/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

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
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
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
            inodos[j].tipo = 'l';   //Señalamos tipo "l" ---> libre

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



int escribir_bit(unsigned int nbloque, unsigned int bit){

    struct superbloque SB;
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    unsigned char bufferMB[BLOCKSIZE];

    //Leemos superbloque
    if(bread(posSB,&SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Calculamos posición byte
    posbyte = nbloque / 8;
    //Calculamos posición bit dentro del byte
    posbit = nbloque % 8;

    //Hallamos en qué bloque se halla ese bit
    nbloqueMB = posbyte / BLOCKSIZE;
    //Obtenemos la posición absoluta
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    //Localizamos la posición
    posbyte = posbyte % BLOCKSIZE;

    //Uso de mascara para poner bits a 0 ó 1
    unsigned char mascara = 128;
    mascara >>= posbit;

    if(bit == 0){
        bufferMB[posbyte] &= ~mascara  // operadores AND y NOT para bits
    } else if(bit == 1){
        bufferMB[posbyte] | = mascara   //  operador OR para bits
    } else {
        fprintf(stderr, "Error en Escribir_bit: parámetro \"bit\" no válido\n");
        return EXIT_FAILURE;
    }

    //Escribimos en el dispositivo virtual
    return bwrite(nbloqueabs, bufferMB);
}



char leer_bit(unsigned int nbloque){

    struct superbloque SB;
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    unsigned char bufferMB[BLOCKSIZE];

    //Leemos superbloque
    if(bread(posSB,&SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }


    //Calculamos posición byte
    posbyte = nbloque / 8;
    //Calculamos posición bit dentro del byte
    posbit = nbloque % 8;

    //Hallamos en qué bloque se halla ese bit
    nbloqueMB = posbyte / BLOCKSIZE;
    //Obtenemos la posición absoluta
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    //Localizamos la posición
    posbyte = posbyte % BLOCKSIZE;

    unsigned char mascara = 128;

    //Leemos posición absoluta
    if(bread(nbloqueabs,bufferMB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Utilizamos desplazamiento a la derecha de bits para leer
    mascara >>= posbit;
    mascara &= bufferMB[posbyte];
    mascara >>= (7 - posbit);

    return mascara;
}



int reservar_bloque(){
    struct superbloque SB;
    int posBloqueMB, posbyte;
    unsigned char bufferAux[BLOCKSIZE];
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128;

    //Leemos superbloque
    if(bread(posSB,&SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Comprobamos la cantidad de bloques libres
    if(SB.cantBloquesLibres <= 0){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con 1s

    //Recorremos bloques del MB
    for(posBloqueMB = SB.posPrimerBloqueMB; posBloqueMB < SB.posUltimoBloqueMB / BLOCKSIZE; posBloqueMB++){

        //Leemos el bloque
        if(bread(posBloqueMB, bufferMB) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }

        //En caso de no ser igual (ya que está todo a 1s y buscamos algún 0), termina el bucle
        if(memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0){
            break;
        }
    }

    //Comprobamos dentro del bloque en que byte se encuentra el 0
    for(posbyte = 0; posbyte <= BLOCKSIZE; posbyte++){
        if(bufferMB[posbyte] != 255){
            break;
        }
    }

    posbit = 0;

    // encontrar el primer bit a 0 en ese byte
    while (bufferMB[posbyte] & mascara) { // operador AND para bits
        bufferMB[posbyte] <<= 1;          // desplazamiento de bits a la izquierda
        posbit++;
    }

    //Para determinar el bloque que podemos reservar
    nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

    //Escribimos para indicar que ese bloque está reservado
    if(escribir_bit(nbloque, 1) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Decrementamos nº bloques libres
    SB.cantBloquesLibres--;

    //Salvamos superbloque
    if(bwrite(0, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Guardamos un buffer de 0s en pos nbloque por si hubiera basura
    memset(bufferAux, 0, BLOCKSIZE);
    if(bwrite(nbloque, bufferAux) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Devolvemos el bloque que se reserva
    return nbloque;
}



int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;

    //Leemos superbloque
    if(bread(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Ponemos a 0 el bloque del MB correspondiente
    if(escribir_bit(nbloque, 0) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Incrementamos la cantidad de libres
    SB.cantBloquesLibres++;

    //Devolvemos el bloque que ha sido liberado
    return nbloque;
}



int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    return 0;
}



int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    return 0;
}



int reservar_inodo(unsigned char tipo, unsigned char permisos){
    return 0;
}