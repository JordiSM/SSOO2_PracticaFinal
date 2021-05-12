/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
    char buf_bloque[BLOCKSIZE];
    unsigned int primerBL, ultimoBL, desp1, desp2 ,nbfisico;
    int bescritos = 0;

    //Leemos el inodo
    if(leer_inodo(ninodo, &inodo) == -1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //Comprobamos en caso de que no sea un permiso de escritura
    if((inodo.permisos & 2) != 2){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
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
        if(bread(nbfisico, buf_bloque) == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return -1;
        }

        //Escribimos en nbytes
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        //Escribimos en el bloque físico
        if(bwrite(nbfisico, buf_bloque) == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return -1;
        }

        //Actualizamos los bytes escritos
        bescritos = desp2 - desp1 + 1;

    }else{ // en caso de que la operación afecte a más de un bloque

        nbfisico = traducir_bloque_inodo(ninodo,primerBL,1);

        //almacenamos el resultado en el buffer
        if(bread(nbfisico, buf_bloque) == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return -1;
        }
        
        //copiamos los restantes bytes a buf_bloque
        memcpy(buf_bloque + desp1, buf_original,BLOCKSIZE - desp1);
    
        //Escritura del buffer en el bloque físico
        if(bwrite(nbfisico, buf_bloque) == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return -1;
        }

        //Bytes escritos
        bescritos = BLOCKSIZE - desp1;

        //Iteramos para cada bloque lógico intermedio i
        for(int i = primerBL + 1; i < ultimoBL;i++){

            nbfisico = traducir_bloque_inodo(ninodo, i, 1);

            //lo volcamos al dispositivo en el bloque físico
            if(bwrite(nbfisico,buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1){
                fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
                return -1;
            }
                
            bescritos += BLOCKSIZE;
        }

        nbfisico = traducir_bloque_inodo(ninodo,ultimoBL,1);

        //Lectura del bloque físico correspondiente y almacenamos el resutado en el buffer
        if(bread(nbfisico,buf_bloque) == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return -1;
        }
        
        //Copiamos los bytes a buf_bloque
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        //Escritura en la posición nbfisico correspondiente a ese bloque lógico
        if(bwrite(nbfisico,buf_bloque) == -1){
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
            return -1;
        }

        bescritos += desp2 + 1;
    }

    //Lectura del inodo actualizado
    if(leer_inodo(ninodo, &inodo) == -1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
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
    if(escribir_inodo(ninodo, inodo) == -1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    //devolvemos los bytes escritos 
    return bescritos; 
    
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){

    struct inodo inodo;
    int primerBL, ultimoBL, desp1, desp2, nbfisico;
    int leidos = 0;

    //Lectura del inodo
    if(leer_inodo(ninodo, &inodo) == -1){
        fprintf(stderr, "Error leer inodo %d: %s\n", errno, strerror(errno));
        return -1;
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
                if(bread(nbfisico,buf_bloque) == -1){
                    fprintf(stderr, "Error nbfisico%d: %s\n", errno, strerror(errno));
                    return -1;
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
                if(bread(nbfisico,buf_bloque) == -1){
                    fprintf(stderr, "Error nbfisico lectura%d: %s\n", errno, strerror(errno));
                    return -1;
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
                    if(bread(nbfisico,buf_bloque) == -1){
                        fprintf(stderr, "Error nbfisico lectura 2%d: %s\n", errno, strerror(errno));
                        return -1;
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
                if(bread(nbfisico,buf_bloque) == -1){
                    fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
                    return -1;
                }
                // Escritura en la posición del bloque correspondiente del buffer original
                memcpy(buf_original + (nbytes - desp2 - 1),buf_bloque, desp2 + 1);
                
            }
            // Actualización de los bytes leídos
            leidos = leidos + desp2 + 1;
        }        

    }else{ // No hay permisos de lectura
        fprintf(stderr, "Error, no hay permisos de lectura %d: %s\n", errno, strerror(errno));
        return -1;
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

    if(leer_inodo(ninodo, &inodo) == -1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
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
    if(leer_inodo(ninodo, &inodo) == -1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
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

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){

    struct inodo inodo;

    //lectura del inodo que nos pasan por parámetro
    if(leer_inodo(ninodo,&inodo) == -1){
        fprintf(stderr,"Error de lectura.\n");
        return -1;
    }

    //Si el inodo tiene permisos de lectura
    if((inodo.permisos & 2) == 2){
        //En caso de sobrepasar el tamaño del fichero en bytes lógicos no se puede truncar
        if(inodo.tamEnBytesLog < nbytes){
            fprintf(stderr,"Error. No se puede truncar.\n");
            return -1;
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
        if( escribir_inodo(ninodo,inodo) == -1){
            fprintf(stderr,"Error de escritura.\n");
            return -1;
        }
        // Devolución de la cantidad de bloques liberados
        fprintf(stdout, "Bloques liberados: %d\n", bloquesLiberados);

        return bloquesLiberados;
    } else {
        fprintf(stderr, "Error de permisos de lectura\n");
        return -1;
    }
}