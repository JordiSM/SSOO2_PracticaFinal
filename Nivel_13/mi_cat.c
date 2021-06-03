/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "directorios.h"
#define TAM_BUFFER 1500

int main(int argc, char *argv[]){

    int bytesLeidos, off, totalBytesLeidos;
    unsigned char buffer[TAM_BUFFER];
    
    //Comprobamos el número de argumentos es correcto
    if(argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero> \n");
        return -1;
    }

    //Comprobamos que la ruta corresponde a un fichero
    if(argv[2][strlen(argv[2]) - 1] == '/') {
        fprintf(stderr, "Error: %s es un directorio.\n", argv[2]);
        return EXIT_FAILURE;
    }

    //Montamos dispositivo
    if(bmount(argv[1]) == -1) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }

    //Inicializamos variables y buffer a 0
    off = 0;
    totalBytesLeidos = 0;
    memset(buffer, 0, TAM_BUFFER);
    bytesLeidos = mi_read(argv[2], buffer, off, TAM_BUFFER);

    //Bucle para leer mientras queden por leer
    while(bytesLeidos > 0) {
        write(1, buffer, bytesLeidos);
        memset(buffer, 0, TAM_BUFFER);
        off += TAM_BUFFER;
        totalBytesLeidos += bytesLeidos;
        bytesLeidos = mi_read(argv[2], buffer, off, TAM_BUFFER);
    }

    fprintf(stderr, "\nTotal de Bytes leidos: %d\n", totalBytesLeidos);

    if(bumount() == -1) {
        fprintf(stderr, "Error en bumount de mi_cat.c\n");
        return -1;
    }
}



