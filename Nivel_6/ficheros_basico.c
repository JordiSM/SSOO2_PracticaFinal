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
    
    struct superbloque SB;
    int i, j, k;

    if (bread(0, &SB) == 1) return EXIT_FAILURE; // Leemos superbloque
    for (i = 0; i < SB.posPrimerBloqueMB; i++) { // Superbloque
        if (escribir_bit(i, 1) == 1) return EXIT_FAILURE;
        SB.cantBloquesLibres--;
    }
    for (j = SB.posPrimerBloqueMB; j <= SB.posUltimoBloqueMB; j++) { // Mapa de bits
        if (escribir_bit(j, 1) == 1) return EXIT_FAILURE;
        SB.cantBloquesLibres--;
    }
    for (k = SB.posPrimerBloqueAI; k <= SB.posUltimoBloqueAI; k++) { // Array de inodos
        if (escribir_bit(k, 1) == 1) return EXIT_FAILURE;
        SB.cantBloquesLibres--;
    }

    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, 0, sizeof(bufferMB));

    for(int i = SB.posPrimerBloqueDatos; i <= SB.posUltimoBloqueDatos; i++){
        if (escribir_bit(i, 0) == 1) return EXIT_FAILURE;
        if (bwrite(i,&bufferMB)== 0){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    return bwrite(0, &SB);
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
    if (bread(nbloqueabs, bufferMB) == -1)
        return -1;
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
    for(posBloqueMB = SB.posPrimerBloqueMB; posBloqueMB <= SB.posUltimoBloqueMB; posBloqueMB++){

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
    for(posbyte = 0; posbyte < BLOCKSIZE; posbyte++){
        if(bufferMB[posbyte] < 255){
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

    //printf("posbit: %d \n\n", posbit);
    //printf("posbyte: %d \n\n", posbyte);
    //printf("%d \n\n", nbloque);
    //printf("%d \n\n", posBloqueMB);

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
    if(bwrite(nbloque, bufferAux) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Devolvemos el bloque que se reserva
    //return SB.posPrimerBloqueDatos + nbloque;
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

    //Cargamos en el superbloque
    if(bread(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Comprobamos que haya libres
    if(SB.cantInodosLibres <= 0){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Obtenemos primer inodo libre
    posInodoReservado = SB.posPrimerInodoLibre;
   
    if(leer_inodo(posInodoReservado, &inodos) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    SB.posPrimerInodoLibre = inodos.punterosDirectos[0];

    //inicilizar todos los campos del superbloque
    inodos.tipo = (char) tipo;
    inodos.permisos = permisos;
    inodos.nlinks = 1;
    inodos.tamEnBytesLog = 0;
    inodos.atime = time(NULL);
    inodos.mtime = time(NULL);
    inodos.ctime = time(NULL);
    inodos.numBloquesOcupados = 0; 

    //Inicializamos punteros
    for(int i = 0; i < 12; i++){
        inodos.punterosDirectos[i] = 0;
    }

    for(int i = 0; i < 3; i++){
        inodos.punterosIndirectos[i] = 0;
    }

    //Decrementamos la cantidad de libres
    SB.cantInodosLibres--;

    //escribir el inodo inicializado en la posición en la que estaba el primer inodo libre
    if(escribir_inodo(posInodoReservado, inodos) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Reescritura del superbloque
    if(bwrite(posSB, &SB) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    return posInodoReservado;
}

int obtener_nRangoBL (struct inodo inodo, unsigned int nblogico, unsigned int *ptr){
    if(nblogico < DIRECTOS){
        *ptr = inodo.punterosDirectos[nblogico];
        return 0;
    }else if(nblogico < INDIRECTOS0){
        *ptr = inodo.punterosIndirectos[0];
        return 1;
    }else if(nblogico < INDIRECTOS1){
        *ptr = inodo.punterosIndirectos[1];
        return 2;
    }else if(nblogico < INDIRECTOS2){
        *ptr = inodo.punterosIndirectos[2];
        return 3;
    }else{
        *ptr = 0;
        fprintf(stderr, "Bloque lógico fuera de rango \n");
        return -1;
    }
}

int obtener_indice (unsigned int nblogico, unsigned int nivel_punteros){
    
    if(nblogico < DIRECTOS){

        return nblogico;
    
    }else if(nblogico < INDIRECTOS0){
        
        return nblogico - DIRECTOS;
    
    }else if(nblogico < INDIRECTOS1){
    
        if(nivel_punteros == 2){
    
            return (nblogico - INDIRECTOS0)/NPUNTEROS;
    
        }else if(nivel_punteros == 1){
    
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
    
        }
    
    }else if(nblogico < INDIRECTOS2){
    
        if(nivel_punteros == 3){
    
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
    
        }else if(nivel_punteros == 2){
    
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
    
        }else if(nivel_punteros == 1){
    
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
    
        }
    }

    return -1;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
    struct inodo inodo;
    unsigned int ptr;
    int ptr_ant;
    int salvar_inodo;
    int nRangoBL;
    int nivel_punteros;
    int indice;
    int buffer[NPUNTEROS];

    //Lectura del inodo
    if (leer_inodo(ninodo, &inodo) == 1) {
        printf("Error en lee_inodo\n");
        return EXIT_FAILURE;
    };

    ptr = ptr_ant = salvar_inodo = 0;

    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL;

    while(nivel_punteros > 0){

        if(ptr == 0){
            if(reservar == 0){
                return -1;
            }else{
               salvar_inodo = 1;
               ptr = reservar_bloque(); //de punteros 
               //printf("Caso 1 ptr = 0, reservamos : %d \n", ptr);        
               inodo.numBloquesOcupados++;
               inodo.ctime = time(NULL); //fecha actual
               if(nivel_punteros == nRangoBL ){
                    inodo.punterosIndirectos[nRangoBL-1] = ptr;
                    printf("[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para BL %d)]\n", nRangoBL-1, inodo.punterosIndirectos[nRangoBL-1], inodo.punterosIndirectos[nRangoBL-1], nblogico);
               } else {
                    buffer[indice] = ptr; // (imprimirlo para test)
                    printf("[traducir_bloque_inodo()→ punteros_nivel%d[%d] = %d (reservado BF %d para BL %d)]\n", nivel_punteros, indice, buffer[indice], buffer[indice], nblogico);
                    if(bwrite(ptr_ant, buffer) == EXIT_FAILURE){
                        fprintf(stderr, "Error en bwrite() en traducir_bloque_inodo %d: %s\n", errno, strerror(errno));
                        return EXIT_FAILURE;
                    }   
               }
            }
        }
        
        if(bread(ptr, buffer) == 1){
            printf("Error bread() en traducir_bloque_inodo");
            return(EXIT_FAILURE);
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }

    if(ptr == 0){
        if(reservar == 0){
            return -1;
        }else{
        
            salvar_inodo = 1;
            ptr = reservar_bloque();
            //printf("Caso 2 ptr = 0, reservamos : %d \n", ptr);
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            if(nRangoBL == 0){
                inodo.punterosDirectos[nblogico] = ptr;
                printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n", nblogico, inodo.punterosDirectos[nblogico], inodo.punterosDirectos[nblogico], nblogico);
            } else {
                buffer[indice] = ptr;
                printf("[traducir_bloque_inodo()→ punteros_nivel%d[%d] = %d (reservado BF %d para BL %d)]\n", nivel_punteros, indice, buffer[indice], buffer[indice], nblogico);
                if(bwrite(ptr_ant, buffer) == EXIT_FAILURE){
                    printf("Error en bwrite de traducir_bloque_inodo()\n");
                    return -1;
                }
            }
        }
    }
    
     if(salvar_inodo==1){
         if(escribir_inodo(ninodo, inodo)==1){
             printf("Error en escribir_inodo de traducir_bloque_inodo()\n");
             return -1;
         } //sólo si lo hemos actualizado
     }

    return ptr;
}

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
    char buf_bloque[BLOCKSIZE];
    unsigned int primerBL, ultimoBL, desp1, desp2 ,nbfisico;
    int bescritos = 0;

    //Leemos el inodo
    if(leer_inodo(ninodo, &inodo) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Comprobamos en caso de que no sea un permiso de escritura
    if((inodo.permisos & 2) != 2){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Obtenemos primer bloque lógico
    primerBL = offset /BLOCKSIZE;
    //Obtenemos último bloque lógico
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    //Calculamos desplazamiento en el bloque
    desp1 = offset % BLOCKSIZE;
    //Calculamos desplazamiento en el bloque para ber los nbytes
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Comprobamos si cabe en un solo bloque
    if(primerBL == ultimoBL){

        //Obtenemos nº bloque físico
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);

        //Leemos el bloque físico
        if(bread(nbfisico, buf_bloque) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }

        //Escribimos en nbytes
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        //Escribimos en el bloque físico
        if(bwrite(nbfisico, buf_bloque) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }

        //Actualizamos los bytes escritos
        bescritos = desp2 - desp1 + 1;

    }else{ // en caso de que la operación afecte a más de un bloque

        nbfisico = traducir_bloque_inodo(ninodo,primerBL,1);

        //almacenamos el resultado en el buffer
        if(bread(nbfisico, buf_bloque) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
        
        //copiamos los restantes bytes a buf_bloque
        memcpy(buf_bloque + desp1, buf_original,BLOCKSIZE - desp1);
    
        //Escritura del buffer en el bloque físico
        if(bwrite(nbfisico, buf_bloque) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }

        //Bytes escritos
        bescritos = BLOCKSIZE - desp1;

        //Iteramos para cada bloque lógico intermedio i
        for(int i = primerBL + 1; i < ultimoBL;i++){

            nbfisico = traducir_bloque_inodo(ninodo, i, 1);

            //lo volcamos al dispositivo en el bloque físico
            if(bwrite(nbfisico,buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == 1){
                fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
                return EXIT_FAILURE;
            }
                
            bescritos += BLOCKSIZE;
        }

        nbfisico = traducir_bloque_inodo(ninodo,ultimoBL,1);

        //Lectura del bloque físico correspondiente y almacenamos el resutado en el buffer
        if(bread(nbfisico,buf_bloque) == 1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
        
        //Copiamos los bytes a buf_bloque
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        //Escritura en la posición nbfisico correspondiente a ese bloque lógico
        if(bwrite(nbfisico,buf_bloque) == 11){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }

        bescritos += desp2 + 1;
    }

    //Lectura del inodo actualizado
    if(leer_inodo(ninodo, &inodo) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Actualizamos el tamaño en bytes lógico, tamEnBytesLog, solo si hemos escrito más allá del final del fichero
    if(offset + nbytes > inodo.tamEnBytesLog){ 
        inodo.tamEnBytesLog = offset + nbytes;
        //actualizar ctime
        inodo.ctime = time(NULL);
    }

    //actualizar
    inodo.mtime = time(NULL);
    
    //escribimos en inodo
    if(escribir_inodo(ninodo, inodo) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //devolvemos los bytes escritos 
    return bescritos; 
    
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){

    struct inodo inodo;
    int primerBL, ultimoBL, desp1, desp2, nbfisico;
    int leidos = 0;

    //Lectura del inodo
    if(leer_inodo(ninodo, &inodo) == 1){
        fprintf(stderr, "Error leer inodo %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //averigua si se puede realizar la operación de lectura
    if((inodo.permisos & 4) == 4){
    
        //En caso de que se quiera leer en una posición que no está dentro del EOF
        if(offset >= inodo.tamEnBytesLog){
            return leidos;
        }
        //en caso de que se pretenda leer más allá del EOF  
        if((offset + nbytes) >= inodo.tamEnBytesLog){
            nbytes = inodo.tamEnBytesLog - offset; 
        }

        primerBL = offset /BLOCKSIZE;
        ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

        desp1 = offset % BLOCKSIZE;
        desp2 = (offset + nbytes - 1) % BLOCKSIZE;

        unsigned char buf_bloque[BLOCKSIZE];

        //lectura de un único bloque
        if(primerBL == ultimoBL){ 
            nbfisico = traducir_bloque_inodo(ninodo,primerBL,0); //reservar es igual a 0
            if( nbfisico != -1){ // En caso de que haya un bloque físico
                // Lectura del bloque
                if(bread(nbfisico,buf_bloque) == 1){
                    fprintf(stderr, "Error nbfisico%d: %s\n", errno, strerror(errno));
                    return EXIT_FAILURE;
                }
                //Copiar datos al buffer original
                memcpy(buf_original,buf_bloque + desp1, nbytes);                
            }
            // acumulación de bytes leidos
            leidos = nbytes;
        }else{ // En caso de que haya varios bloques por leer
            // Primer bloque lógico
            nbfisico = traducir_bloque_inodo(ninodo,primerBL,0); //reservar es igual a 0
            if( nbfisico != -1){ // En caso de que haya un bloque físico asignado al bloque lógico
                // Lectura del bloque
                if(bread(nbfisico,buf_bloque) == 1){
                    fprintf(stderr, "Error nbfisico lectura%d: %s\n", errno, strerror(errno));
                    return EXIT_FAILURE;
                }
                //Copiar datos al buffer original
                memcpy(buf_original,buf_bloque + desp1, BLOCKSIZE - desp1);                
            }
            // Acumulación de bytes leidos
            leidos = BLOCKSIZE - desp1;
            // Bloques intermedios
            // Iterar para avanzar en los bloques que hay que escribir
            for(int i = primerBL + 1; i < ultimoBL;i++){
                // Guardamos el bloque fisico asociado al bloque lógico correspondiente
                nbfisico = traducir_bloque_inodo(ninodo,i,0); 
                if( nbfisico != -1){ // En caso de que haya un bloque físico asignado al bloque lógico
                    // Lectura del bloque
                    if(bread(nbfisico,buf_bloque) == 1){
                        fprintf(stderr, "Error nbfisico lectura 2%d: %s\n", errno, strerror(errno));
                        return EXIT_FAILURE;
                    }
                    //Copiar datos al buffer original 
                    memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);                
                }
                // Actualizamos bytes leídos
                leidos = leidos + BLOCKSIZE;
            }
            // Último bloque lógico
            // Guardamos el bloque fisico relacionado con el ninodo pasado por parámetro
            nbfisico = traducir_bloque_inodo(ninodo,ultimoBL,0); 
            if(nbfisico != -1){
                // Lectura del bloque entero para no sobreescribir posibles datos
                if(bread(nbfisico,buf_bloque) == 1){
                    fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
                    return EXIT_FAILURE;
                }
                // Escritura en la posición del bloque correspondiente del buffer original
                memcpy(buf_original + (nbytes - desp2 - 1),buf_bloque, desp2 + 1);
                
            }
            // Actualización de los bytes leídos
            leidos = leidos + desp2 + 1;
        }        

    }else{ // No hay permisos de lectura
        fprintf(stderr, "Error, no hay permisos de lectura %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    
    // Finalmente actualizamos el atime del inodo
    leer_inodo(ninodo, &inodo); // Leemos inodo
	inodo.atime = time(NULL); // Actualizamos el último acceso
	escribir_inodo(ninodo, inodo); // Salvamos el inodo
    
    //devolver bytes leidos
    return leidos;
}

//Cambia los permisos de un fichero/directorio
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    struct inodo inodo;

    if(leer_inodo(ninodo, &inodo) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    inodo.permisos = permisos;

    //actualización
    inodo.ctime = time(NULL);
    
    return escribir_inodo(ninodo, inodo);
}

//Devuelve la metainformación de un fichero/directorio con el valor que indique el argumento permisos
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){

    struct inodo inodo;

    //Lectura inodo
    if(leer_inodo(ninodo, &inodo) == 1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Guardar valores
    p_stat -> tipo = inodo.tipo;
    p_stat -> permisos = inodo.permisos;

    p_stat -> atime = inodo.atime;
    p_stat -> mtime = inodo.mtime;
    p_stat -> ctime = inodo.ctime;

    p_stat -> nlinks = inodo.nlinks;
    p_stat -> tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat -> numBloquesOcupados = inodo.numBloquesOcupados;

    return EXIT_SUCCESS;
}

int liberar_inodo(unsigned int ninodo){
    struct inodo inodo;
    struct superbloque SB;

    //Leemos el inodo
    if(leer_inodo(ninodo,&inodo) == 1){
        fprintf(stderr, "Error leer inodo %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Liberamos los bloques
    int bloquesLibres = liberar_bloques_inodo(0,&inodo); 

    if(bloquesLibres == -1){
        fprintf(stderr, "Error bloques libres%d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Bloques liberados: %d", bloquesLibres);

    //Debería quedar a 0
    inodo.numBloquesOcupados -= bloquesLibres;

    inodo.tipo='l';
    inodo.tamEnBytesLog = 0;

    //Leemos el superbloque
    if(bread(posSB,&SB) == 1){
        fprintf(stderr, "Error en bread%d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Apuntamos al primer inodo libre
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;

    //Incrementamos inodos libres
    SB.cantInodosLibres++;

    //Escribimos en el inodo
    if(escribir_inodo(ninodo,inodo) == 1){
        fprintf(stderr, "Error en escribir inodo%d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    if(bwrite(posSB,&SB) == 1){
        fprintf(stderr, "Error en bwrite %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }  

    return EXIT_SUCCESS;    
}

//libera todos los bloques ocupados
int liberar_bloques_inodo(unsigned int primerBL,struct inodo *inodo){
    unsigned int nivel_punteros, indice, ptr, nBL, ultimoBL;

    int nRangoBL;

    unsigned int bloques_punteros[3][NPUNTEROS];
    unsigned char bufAux_punteros [BLOCKSIZE];

    int ptr_nivel[3];
    int indices[3];
    int liberados = 0;


    liberados= 0;

    if(inodo->tamEnBytesLog == 0){
        return 0;
    }

    //Calcular cual es es el último bloque lógico del fichero
    if((inodo->tamEnBytesLog % BLOCKSIZE) == 0){
        ultimoBL=(inodo->tamEnBytesLog / BLOCKSIZE) - 1;
    } else {
        ultimoBL=(inodo->tamEnBytesLog / BLOCKSIZE);
    }

    memset(bufAux_punteros, 0 , BLOCKSIZE);
    ptr = 0;

    printf("[liberar_bloques_inodo()→ primer BL %d, ultimo BL %d]\n", primerBL, ultimoBL);

    for (nBL = primerBL; nBL <= ultimoBL; nBL++){
        nRangoBL = obtener_nRangoBL(*inodo,nBL, &ptr);
        if(nRangoBL < 0){
            fprintf(stderr, "Error en obtener_nRangoBL");
            return -1; //Modificacion de EXIT FAILURE a -1 por posible retorno de liberados=1
        }

        nivel_punteros = nRangoBL;

        while(ptr >0 && nivel_punteros >0){
            indice = obtener_indice(nBL, nivel_punteros);
            if(indice==0||nBL==primerBL){
                
                if(bread(ptr, bloques_punteros[nivel_punteros-1])==EXIT_FAILURE){
                    fprintf(stderr,"Error en bread");
                    return -1; //Modificacion de EXIT FAILURE a -1 por posible retorno de liberados=1
                }
            }

            ptr_nivel[nivel_punteros-1]= ptr;
            indices[nivel_punteros-1]= indice;
            ptr= bloques_punteros[nivel_punteros-1][indice];
            nivel_punteros--;
        }

        if(ptr > 0){
            liberar_bloque(ptr);
            liberados++;
            if(nRangoBL == 0){
                inodo->punterosDirectos[nBL] = 0;
            } else{
                nivel_punteros = 1;
                printf("[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d)]\n", ptr, nBL);
    
                while(nivel_punteros <= nRangoBL){
                    indice = indices[nivel_punteros-1];
                    bloques_punteros[nivel_punteros-1][indice] = 0;
                    ptr=ptr_nivel[nivel_punteros-1];
                    
                    if(memcmp(bloques_punteros[nivel_punteros-1], bufAux_punteros, BLOCKSIZE) == 0){

                        liberar_bloque(ptr);
                        liberados++;

                        printf("[liberar_bloques_inodo()→ liberado BF = %d nivel_punteros%d correspondiente BL %d)]\n", ptr, nivel_punteros, nBL);

                        if(nivel_punteros == nRangoBL){
                            inodo->punterosIndirectos[nRangoBL-1]= 0;
                        }

                        nivel_punteros++;

                    }else{

                        bwrite(ptr, bloques_punteros[nivel_punteros-1]);
                        //COLOCAR MARGEN DE ERROR
                        nivel_punteros= nRangoBL+1;
                    }
                }
                
            }

        }
   }

    printf("[liberar_bloques_inodo()→ total bloques liberados: %d\n", liberados);
    return liberados;
}
