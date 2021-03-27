/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi

  FORMATO DE USO:

 ./mi_mkfs "nombre_de_memoria" "numero_de_bloques" #COMENTARIOS
 ./leer_sf disco > archivo.txt
*/

#include "ficheros_basico.h"

int main(int argc, char **argv){

    if(argc < 2){
        printf("Error, número de argumentos no válidos\n");
        printf("Formato de uso :\n\t./mi_mkfs \"nombre_de_memoria\" \"numero_de_bloques\" #COMENTARIOS\n");
        fflush(stdout);
        exit(EXIT_FAILURE);

    }else{

        if(bmount(argv[1])==1){
            exit(EXIT_FAILURE);
        }

        struct superbloque sb;
        //struct inodo inodos[BLOCKSIZE/INODOSIZE];

        if(bread(0, &sb) == 1){
            exit(EXIT_FAILURE);
        }

        printf ("DATOS DEL SUPERBLOQUE\n");
        printf ("posPrimerBloqueMB = %u\n", sb.posPrimerBloqueMB);                 // Posición del primer bloque del mapa de bits
        printf ("posUltimoBloqueMB = %u\n", sb.posUltimoBloqueMB);                 // Posición del último bloque del mapa de bits
        printf ("posPrimerBloqueAI = %u\n", sb.posPrimerBloqueAI);                 // Posición del primer bloque del array de inodos
        printf ("posUltimoBloqueAI = %u\n", sb.posUltimoBloqueAI);                 // Posición del último bloque del array de inodos
        printf ("posPrimerBloqueDatos = %u\n", sb.posPrimerBloqueDatos);           // Posición del primer bloque de datos
        printf ("posUltimoBloqueDatos = %u\n", sb.posUltimoBloqueDatos);           // Posición del último bloque de datos
        printf ("posInodoRaiz; = %u\n", sb.posInodoRaiz);                          // Posición del inodo del directorio raíz
        printf ("posPrimerInodoLibre = %u\n", sb.posPrimerInodoLibre);             // Posición del primer inodo libre
        printf ("cantBloquesLibres = %u\n", sb.cantBloquesLibres);                 // Cantidad de bloques libres
        printf ("cantInodosLibres = %u\n", sb.cantInodosLibres);                   // Cantidad de inodos libres
        printf ("totBloques = %u\n", sb.totBloques);                               // Cantidad total de bloques
        printf ("totInodos; = %u\n", sb.totInodos);   
        printf ("\n");
        
        printf ("sizeof struct superbloque is: %lu\n", sizeof(struct superbloque));
        printf ("sizeof struct inodo is: %lu\n", sizeof(struct inodo));
        printf ("\n\n");
        
        /*
        printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
        for(int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++){
            if(bread(i, &inodos)==1){
                return EXIT_FAILURE;
            }

            for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++){
                printf("%d ", inodos[j].punterosDirectos[0]);
            }
            printf("\n");
            fflush(stdout);
        }
        */

        printf ("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
        
        unsigned int bloqueReservado = reservar_bloque();
        if(bloqueReservado == EXIT_FAILURE){
            exit(EXIT_FAILURE);
        }
        printf ("Se ha reservado el bloque físico nº %u que era el %uº libre indicado por el MB\n", bloqueReservado, (bloqueReservado - sb.posUltimoBloqueAI));
       fflush(stdout);
        if(bread(0, &sb) == 1){
            exit(EXIT_FAILURE);
        }

        printf ("cantBloquesLibres = %u\n", sb.cantBloquesLibres);                 // Cantidad de bloques libres
        
        printf ("Liberamos ese bloque\n");
        if(liberar_bloque(bloqueReservado) == EXIT_FAILURE){
            exit(EXIT_FAILURE);
        }
        
        if(bread(0, &sb) == 1){
            exit(EXIT_FAILURE);
        }

        printf ("cantBloquesLibres = %u\n", sb.cantBloquesLibres);                 // Cantidad de bloques libres
       fflush(stdout);

        printf ("DATOS DEL DIRECTORIO RAIZ\n");
        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];

        struct inodo innodo;
        int ninodo = 1;
        
        leer_inodo(ninodo, &innodo);
        printf ("tipo: %u\n", innodo.tipo);
        printf ("permisos: %u\n", innodo.permisos);

        ts = localtime(&innodo.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&innodo.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&innodo.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf("ID: %d\n ATIME: %s\n MTIME: %s\n CTIME: %s\n",ninodo,atime,mtime,ctime);
        
        printf ("nlinks: %u\n", innodo.nlinks);
        printf ("tamEnBytesLog: %u\n", innodo.tamEnBytesLog);
        printf ("nbloquesOcupados: %u\n", innodo.numBloquesOcupados);
        bumount();
    }
}