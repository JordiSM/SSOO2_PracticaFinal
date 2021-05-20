#include "directorios.h"
#define TAM_BUFFER 1500

int main(int argc, char *argv[]){

    //char *ruta, *nomDispositivo;
    int bytesLeidos, off, totalBytesLeidos;
    unsigned char buffer[TAM_BUFFER];
    
    if(argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero> \n");
        return -1;
    }

    //nomDispositivo = argv[1];
    //ruta = argv[2];

    if(argv[2][strlen(argv[2]) - 1] == '/') {
        fprintf(stderr, "Error: %s es un directorio.\n", argv[2]);
        return EXIT_FAILURE;
    }

    if(bmount(argv[1]) == -1) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }

    off = 0;
    totalBytesLeidos = 0;
    memset(buffer, 0, TAM_BUFFER);
    bytesLeidos = mi_read(argv[2], buffer, off, TAM_BUFFER);

    while(bytesLeidos > 0) {
        write(1, buffer, bytesLeidos);
        memset(buffer, 0, TAM_BUFFER);
        off += TAM_BUFFER;
        totalBytesLeidos += bytesLeidos;
        bytesLeidos = mi_read(argv[2], buffer, off, TAM_BUFFER);
    }

    printf("\nTotal de Bytes leidos: %d\n", totalBytesLeidos);

    if(bumount() == -1) {
        fprintf(stderr, "Error en bumount de mi_cat.c\n");
        return -1;
    }
}



