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
    SB.cantBloquesLibres = nbloques - SB.posPrimerBloqueDatos;
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

    //1 bit --> 1 bloque
    //1 byte --> 8 bloques
    //3139 bloques son ??
    // 3139 / 8 = 392
    // 3139 % 8 = 3
    
    /*
    for(int i = posSB; i <= posSB; i++){
        if(escribir_bit(i,1) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    for(int i = SB.posPrimerBloqueMB; i < SB.posUltimoBloqueMB; i++){
        if(escribir_bit(i,1) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }

        if(bwrite(i,&bufferMB)== 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    for(int i = SB.posPrimerBloqueAI; i < SB.posUltimoBloqueAI; i++){
        if(escribir_bit(i,1) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
    }*/
    
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
        bufferMB[posbyte] &= ~mascara;  // operadores AND y NOT para bits
    } else if(bit == 1){
        bufferMB[posbyte] |= mascara;   //  operador OR para bits
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
    unsigned int posBloqueMB, posbyte;
    unsigned char bufferAux[BLOCKSIZE];
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128;

    //Leemos superbloque
    if(bread(posSB,&SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Comprobamos la cantidad de bloques libres
    if(SB.cantBloquesLibres <= 0){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con 1s

    //Recorremos bloques del MB
    for(posBloqueMB = SB.posPrimerBloqueMB; posBloqueMB < SB.posUltimoBloqueMB / BLOCKSIZE; posBloqueMB++){

        //Leemos el bloque
        if(bread(posBloqueMB, bufferMB) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return -1;
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

    unsigned int posbit = 0;

    // encontrar el primer bit a 0 en ese byte
    while (bufferMB[posbyte] & mascara) { // operador AND para bits
        bufferMB[posbyte] <<= 1;          // desplazamiento de bits a la izquierda
        posbit++;
    }

    //Para determinar el bloque que podemos reservar
    unsigned int nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

    //Escribimos para indicar que ese bloque está reservado
    if(escribir_bit(nbloque, 1) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Decrementamos nº bloques libres
    SB.cantBloquesLibres--;

    //Salvamos superbloque
    if(bwrite(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Guardamos un buffer de 0s en pos nbloque por si hubiera basura
    memset(bufferAux, 0, BLOCKSIZE);
    if(bwrite(SB.posPrimerBloqueDatos + nbloque, bufferAux) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Devolvemos el bloque que se reserva
    return SB.posPrimerBloqueDatos + nbloque;
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

    //Guardamos el superbloque
    if(bwrite(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Devolvemos el bloque que ha sido liberado
    return nbloque;
}



int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    struct inodo inodos[BLOCKSIZE/INODOSIZE]; 
    struct superbloque SB;
    int pos_bloque, num_bloque;

    //Leemos superbloque
    if(bread(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Obtenemos nº de bloque del array
    num_bloque = (ninodo * INODOSIZE) / BLOCKSIZE;

    //obtenemos la localización donde queremos escribir el inodo
    pos_bloque = num_bloque + SB.posPrimerBloqueAI;

    if(bread(pos_bloque,inodos) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    //escribir en el buffer el contenido que nos pasan por parámetro
    inodos[ninodo % (BLOCKSIZE/INODOSIZE)] = inodo;

    //escritura del array de inodos en el dispositivo virtual
    return bwrite(pos_bloque, inodos);
}



int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    struct superbloque SB;
    int pos_bloque, num_bloque;

    //Leemos superbloque
    if(bread(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Obtenemos el nº de bloque del array
    num_bloque = (ninodo * INODOSIZE) / BLOCKSIZE;
    //obtenemos la localización donde queremos escribir el inodo
    pos_bloque = num_bloque + SB.posPrimerBloqueAI; 
    
    if(bread(pos_bloque, inodos) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    //apunto con el puntero a la dirección del inodo que deseamos
    *inodo = inodos[ninodo % (BLOCKSIZE/INODOSIZE)];

    //en caso de ir correctamente devovemos 0
    return 0;
}



int reservar_inodo(unsigned char tipo, unsigned char permisos){

    struct superbloque SB;
    struct inodo inodos;
    int posInodoReservado;

    if(bread(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    if(SB.cantInodosLibres <= 0){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    posInodoReservado = SB.posPrimerInodoLibre;
   
    if(leer_inodo(posInodoReservado, &inodos) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    SB.posPrimerInodoLibre = inodos.punterosDirectos[0];

    inodos.tipo = (char) tipo;
    inodos.permisos = permisos;
    inodos.nlinks = 1;
    inodos.tamEnBytesLog = 0;
    inodos.atime = time(NULL);
    inodos.mtime = time(NULL);
    inodos.ctime = time(NULL);
    inodos.numBloquesOcupados = 0; 

    for(int i = 0; i < 12; i++){
        inodos.punterosDirectos[i] = 0;
    }

    for(int i = 0; i < 3; i++){
        inodos.punterosIndirectos[i] = 0;
    }

    SB.cantInodosLibres--;

    if(escribir_inodo(posInodoReservado, inodos) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    if(bwrite(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    return posInodoReservado;
}