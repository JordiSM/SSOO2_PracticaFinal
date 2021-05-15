#include directorios.h
#define TAM_BUFFER 1500

int main(int argc, char *argv[]){

    char *ruta, *nomDispositivo;
    int bytesLeidos, offset, totalBytesLeidos;
    unsigned char buffer[TAM_BUFFER];
    
    if(argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero> \n");
        return -1;
    }

    nomDispositivo = argv[1];
    ruta = argv[2];

    if(ruta[strlen(ruta) - 1] == '/') {
        fprintf(stderr, "Error: %s es un directorio.\n", ruta);
        return -1;
    }

    if(bmount(nombre_dispositivo) == -1) {
        fprintf(stderr, "Error en bmount\n");
        return -1;
    }

    offset = 0;
    totalBytesLeidos = 0;
    memset(buffer, 0, TAM_BUFFER);
    bytesLeidos = mi_read(ruta, buffer, offset, TAM_BUFFER);

    while(bytesLeidos > 0) {
        write(1, buffer, bytesLeidos);
        memset(buffer, 0, TAM_BUFFER);
        offset += TAM_BUFFER;
        totalBytesLeidos += bytesLeidos;
        bytesLeidos = mi_read(ruta, buffer, offset, TAM_BUFFER);
    }

    printf("\Total leidos: %d\n", totalBytesLeidos);

    if(bumount() == -1) {
        fprintf(stderr, "Error en bumount de mi_cat.c\n");
        return -1;
    }



