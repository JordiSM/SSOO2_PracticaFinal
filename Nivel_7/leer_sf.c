/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi

  FORMATO DE USO:

 ./mi_mkfs "nombre_de_memoria" "numero_de_bloques" #COMENTARIOS
 ./leer_sf disco > archivo.txt
*/

#include "directorios.h"
 
void mostrar_buscar_entrada(char *camino, char reservar){
  unsigned int p_inodo_dir = 0;
  unsigned int p_inodo = 0;
  unsigned int p_entrada = 0;
  int error;
  printf("\ncamino: %s, reservar: %d\n", camino, reservar);
  if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
    mostrar_error_buscar_entrada(error);
  }
  printf("**********************************************************************\n");
  return;
}


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
        
        /*
        printf ("sizeof struct superbloque is: %lu\n", sizeof(struct superbloque));// Tamaño del superbloque
        printf ("sizeof struct inodo is: %lu\n", sizeof(struct inodo));            // Tamaño del inodo
        printf ("\n\n");
        */

        //Mostrar creación directorios y errores
        mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
        mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
        mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
        mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
        mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
        mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  

        //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
        mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
        mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
        mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2

        //Desmontamos el dispositivo
       bumount();
    }
}