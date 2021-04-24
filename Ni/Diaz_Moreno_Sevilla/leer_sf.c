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

    //En caso de que el número de argumentos introducidos por el terminal sea menor que 2
    if(argc < 2){
        fprintf(stderr, "Error, número de argumentos no válidos\n");
        fprintf( stderr, "Formato de uso :\n\t./mi_mkfs \"nombre_de_memoria\" \"numero_de_bloques\" #COMENTARIOS\n");
        exit(EXIT_FAILURE);

    }else{
        //montamos en el dispositivo
        if(bmount(argv[1])==1){
            exit(EXIT_FAILURE);
        }

        struct superbloque sb;

        //leemos el superbloque
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
        printf ("posInodoRaiz = %u\n", sb.posInodoRaiz);                           // Posición del inodo del directorio raíz
        printf ("posPrimerInodoLibre = %u\n", sb.posPrimerInodoLibre);             // Posición del primer inodo libre
        printf ("cantBloquesLibres = %u\n", sb.cantBloquesLibres);                 // Cantidad de bloques libres
        printf ("cantInodosLibres = %u\n", sb.cantInodosLibres);                   // Cantidad de inodos libres
        printf ("totBloques = %u\n", sb.totBloques);                               // Cantidad total de bloques
        printf ("totInodos = %u\n", sb.totInodos);                                 // Cantidad total de inodos
        printf ("\n");
        
        printf ("sizeof struct superbloque is: %lu\n", sizeof(struct superbloque));// Tamaño del superbloque
        printf ("sizeof struct inodo is: %lu\n", sizeof(struct inodo));            // Tamaño del inodo
        printf ("\n\n");
        
        //Desmontamos el dispositivo
       bumount();
    }
}